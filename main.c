#include <stdio.h>

#include "scheduler.h"

#define N 100                         /* nombre de places dans le tampon */

struct sem_s *mutex, *vide, *plein;

struct objet_s {
  int created ;
};

typedef struct objet_s objet_t ;

objet_t lesObjets[100] ;
int indice = 0 ; 

void mettre_objet(objet_t objet){
  lesObjets[indice] = objet ;
  indice ++ ;
  return ;
}

void produire_objet(objet_t* objet){
  objet -> created = 1 ;
  return ;
}

void retirer_objet(objet_t* objet){
  objet->created=lesObjets[indice].created ;
  indice -- ;
  return ;
}

void utiliser_objet(objet_t objet){
  objet.created = 0 ;
  return ;
}

void producteur ()
{
  objet_t objet ;

  while (1) {
    produire_objet(&objet);           /* produire l'objet suivant */
    sem_down(vide);                  /* dec. nb places libres */
    sem_down(mutex);                 /* entree en section critique */
    mettre_objet(objet);              /* mettre l'objet dans le tampon */
    sem_up(mutex);                   /* sortie de section critique */
    sem_up(plein);                   /* inc. nb place occupees */
  }
}

void consommateur ()
{
  objet_t objet ;

  while (1) {
    sem_down(plein);                 /* dec. nb emplacements occupes */
    sem_down(mutex);                 /* entree section critique */
    retirer_objet (&objet);           /* retire un objet du tampon */
    sem_up(mutex);                   /* sortie de la section critique */
    sem_up(vide);                    /* inc. nb emplacements libres */
    utiliser_objet(objet);            /* utiliser l'objet */
  }
}

int main(void) {
  sem_init(mutex, 1);                /* controle d'acces au tampon */
  sem_init(vide, N);                 /* nb de places libres */
  sem_init(plein, 0);                /* nb de places occupees */
  create_ctx(16384, consommateur, NULL);
  create_ctx(16384, producteur, NULL);
  start_sched();

  return 0;
}
