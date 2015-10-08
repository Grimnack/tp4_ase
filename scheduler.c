/*
ASE TP3: ordonancement.
Auteurs: Mathieu Caron, Antoine Amara.
*/

#include <stdlib.h>
#include <assert.h>

#include "scheduler.h"
#include "hw.h"

/* le contexte courant */
static struct ctx_s *current_ctx = NULL;
/* la liste chainée permettant de gérer l'ordonancement */
static struct ctx_s *ring = NULL;

void sem_init(struct sem_s *sem, unsigned int val) {
  sem->sem_cpt = val;
  sem->ctx_block = NULL;
}

void sem_down(struct sem_s *sem) {
  sem->sem_cpt--;

  if(sem->sem_cpt < 0) {
    current_ctx->ctx_state = BLOCK;
    current_ctx->sem_next = sem->ctx_block;
    sem->ctx_block = current_ctx;
    yield();
  }
}

void sem_up(struct sem_s *sem) {
  sem->sem_cpt++;

  if(sem->sem_cpt <= 0) {
    sem->ctx_block->ctx_state = ACTIVABLE;
    sem->ctx_block = sem->ctx_block->sem_next;
  }
}

/* Permet d'initialiser le hardware et de demarrer l'ordonancement en effectuant
le premier yield */
void start_sched() {

  setup_irq(TIMER_IRQ, yield);
  start_hw();
  yield();
}

/* Permet de créer un contexte, c'est à dire d'allouer la mémoire pour celui-ci
et d'effectuer son initialisation.
stack_size: la taille de la pile du contexte
f: la fonction à executer dans le contexte
args: un pointeur vers les eventuelles arguments de la fonction f */
int create_ctx(int stack_size, func_t f, void* args) {
  struct ctx_s *new_ctx;

  irq_disable();

  new_ctx = malloc(sizeof(struct ctx_s));
  init_ctx(new_ctx, stack_size, f, args);

  if(ring == NULL) {
    ring = new_ctx;
    ring->ctx_next = new_ctx;
  }
  else {
    new_ctx->ctx_next = ring->ctx_next;
    ring->ctx_next = new_ctx;
  }
  irq_enable();
  return 0;
}

/* Permet de passer au contexte suivant dans le liste d'ordonancement,
elle lance le saut vers le contexte suivant. */
void yield() {
  struct ctx_s *tmp;
  struct ctx_s *ctx_activable;

  ctx_activable = current_ctx->ctx_next;

  irq_disable();

  if(current_ctx == NULL)
  switch_to_ctx(ring);
  else {
    while((ctx_activable->ctx_state == TERMINATED || ctx_activable->ctx_state == BLOCK) && ctx_activable != current_ctx) {
      while(current_ctx->ctx_next->ctx_state == TERMINATED && current_ctx->ctx_next != current_ctx) {
        free(current_ctx->ctx_next->ctx_stack);
        tmp = current_ctx->ctx_next->ctx_next;
        free(current_ctx->ctx_next);
        current_ctx = tmp;
      }
      if(current_ctx->ctx_next->ctx_state == TERMINATED)
      exit(0);
    }
    if(ctx_activable->ctx_state == TERMINATED || ctx_activable->ctx_state == BLOCK)
    assert(0);

    switch_to_ctx(current_ctx->ctx_next);
    return;
  }
}


/*
Permet d'initialiser une pile
pctx: structure représentant un contexte qui s'executera sur la pile.
stack_size: taille mémoire de la pile.
f: fonction à exécuter dans le contexte.
args: pointeur vers les arguments de la fonction.
*/
int init_ctx(struct ctx_s* pctx, int stack_size, func_t f, void* args) {

  pctx->ctx_stack = malloc(stack_size);

  pctx->ctx_ebp = pctx->ctx_stack + stack_size - sizeof(void*);
  pctx->ctx_esp = pctx->ctx_stack + stack_size - sizeof(void*);
  pctx->ctx_f = f;
  pctx->ctx_args = args;
  pctx->ctx_state = INITIALIZED;
  pctx->ctx_magic = 0xDEADBEEF;

  return 0;
}

/*
Permet de sauter d'un contexte à l'autre. Si aucun contexte n'existe un nouveau sera crée.
pctx: la structure représentant le contexte vers lequel ont va sauter.
*/
void switch_to_ctx (struct ctx_s* pctx){
  /*Premiere etape, sort-on d'un contexte
  si oui on sauvegarde le contexte qu'on a quitté*/
  if(current_ctx!=NULL){
    asm("movl %%ebp, %0 \n\t movl %%esp, %1"
    :"=r" (current_ctx->ctx_ebp),
    "=r" (current_ctx->ctx_esp));
  }
  current_ctx = pctx;
  /*Deuxieme etape, on change de contexte*/
  asm("movl %0, %%ebp \n\t movl %1, %%esp"
  ::"r" (pctx->ctx_ebp),
  "r" (pctx->ctx_esp));
  if(current_ctx->ctx_state==INITIALIZED){
    current_ctx->ctx_state=ACTIVABLE;
    irq_enable();
    (*(current_ctx->ctx_f))(current_ctx->ctx_args);
    return ;
  };
  irq_enable();
  return ;

}
