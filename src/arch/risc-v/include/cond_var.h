#ifndef _COND_VAR_H_
#define _COND_VAR_H_

#include "defs.h"
#include "types.h"

typedef struct n {
  usize item;
  struct n *next;
} node;

typedef struct {
  node *head;
  node *tail;
} queue;

void push_back(queue *q, usize data);
usize pop_front(queue *q);
int is_empty(queue *q);

typedef struct {
  queue wait_queue;
} cond_var;

void wait_condition(cond_var *self);
void notify_condition(cond_var *self);

#endif // _COND_VAR_H_