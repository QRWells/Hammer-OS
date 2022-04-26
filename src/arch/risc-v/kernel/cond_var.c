#include "cond_var.h"

void push_back(queue *q, usize data) {
  node *n = kalloc(sizeof(node));
  n->item = data;
  if (q->head == q->tail && q->head == 0) {
    q->head = n;
    q->tail = n;
  } else {
    q->tail->next = n;
    q->tail = n;
  }
}

usize pop_front(queue *q) {
  node *n = q->head;
  usize ret = n->item;
  if (q->head == q->tail) {
    q->head = 0;
    q->tail = 0;
  } else {
    q->head = q->head->next;
  }
  kfree(n);
  return ret;
}

int is_empty(queue *q) { return (q->head == q->tail && q->head == 0); }

void wait_condition(cond_var *self) {
  push_back(&self->wait_queue, get_current_tid());
  yield_cpu();
}

void notify_condition(cond_var *self) {
  if (!is_empty(&self->wait_queue)) {
    int tid = (int)pop_front(&self->wait_queue);
    wakeup_cpu(tid);
  }
}