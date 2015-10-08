#ifndef SCHEDULER_H
#define SCHEDULER_H

typedef void (func_t) (void*);

enum ctx_state_e {INITIALIZED, ACTIVABLE, TERMINATED, BLOCK};

struct ctx_s {
  void* ctx_ebp;
  void* ctx_esp;
  func_t* ctx_f;
  void * ctx_args;
  enum ctx_state_e ctx_state;
  int ctx_magic;
  struct ctx_s *ctx_next;
  char* ctx_stack;
  struct ctx_s *sem_next;
};

struct sem_s {
  int sem_cpt;
  struct ctx_s* ctx_block;
};

extern void sem_init(struct sem_s *sem, unsigned int val);
extern void sem_down(struct sem_s *sem);
extern void sem_up(struct sem_s *sem);
extern void start_sched();
extern int create_ctx(int stack_size, func_t f, void* args);
extern void yield();
extern int init_ctx(struct ctx_s* pctx, int stack_size, func_t f, void* args);
extern void switch_to_ctx(struct ctx_s* pctx);

#endif
