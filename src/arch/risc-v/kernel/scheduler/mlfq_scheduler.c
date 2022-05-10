#include "types.h"
#include "constants.h"
#include "defs.h"
#include "linked_list.h"

#define LEVEL 3
#define MLFQ_PROMOTION_THRESHOLD 2

typedef struct {
  int tid;
  int time;
} tinfo;

struct {
  linked_list queues[LEVEL];
  int current;
  int max_time[LEVEL];
  int promotion_time;
} mlfq_scheduler;

void promote_all() {
  for (int i = 1; i < LEVEL; i++) {
    linked_list_node *node =
        linked_list_remove_first(&mlfq_scheduler.queues[i]);
    while (node != NULL) {
      linked_list_add_last(&mlfq_scheduler.queues[0], node);
      node = linked_list_remove_first(&mlfq_scheduler.queues[i]);
    }
  }
}

int get_priority(int tid) {
  for (int i = 0; i < LEVEL; ++i) {
    linked_list_node *node = mlfq_scheduler.queues[i].first;
    while (node != NULL) {
      if (((tinfo *)node->data)->tid == tid)
        return i;
      node = node->next;
    }
  }
  return LEVEL + 1;
}

linked_list_node *find(int tid) {
  for (int i = 0; i < LEVEL; ++i) {
    linked_list_node *node = mlfq_scheduler.queues[i].first;
    while (node != NULL) {
      if (((tinfo *)node->data)->tid == tid)
        return node;
      node = node->next;
    }
  }
  return NULL;
}

void level_down(int tid) {
  int priority = get_priority(tid);
  if (priority >= LEVEL - 1)
    return;
  linked_list_node *node = find(tid);
  linked_list_remove(&mlfq_scheduler.queues[priority], node);
  linked_list_add_last(&mlfq_scheduler.queues[priority + 1], node);
}

void scheduler_init() {
  for (int i = 0; i < LEVEL; i++) {
    mlfq_scheduler.queues[i] = *linked_list_init();
    mlfq_scheduler.max_time[i] = 1;
  }
  mlfq_scheduler.current = -1;
  mlfq_scheduler.promotion_time = MLFQ_PROMOTION_THRESHOLD; // magic number
}

void scheduler_push(int tid) {
  if (mlfq_scheduler.queues[0].size >= MAX_THREAD)
    panic("queue is full!\n");

  tinfo *t = kalloc(sizeof(tinfo));
  t->tid = tid;
  t->time = mlfq_scheduler.max_time[0];
  linked_list_node *node = linked_list_create_node((u64)t);
  linked_list_add_last(&mlfq_scheduler.queues[0], node);
}

int scheduler_pop() {
  linked_list_node *node = linked_list_remove_first(&mlfq_scheduler.queues[0]);
  if (node == NULL)
    return -1;
  int tid = ((tinfo *)node->data)->tid;
  mlfq_scheduler.current = tid;
  return tid;
}

int scheduler_tick() {
  mlfq_scheduler.promotion_time--;
  if (mlfq_scheduler.promotion_time == 0) {
    mlfq_scheduler.promotion_time = MLFQ_PROMOTION_THRESHOLD;
    promote_all();
  }
  int tid = mlfq_scheduler.current;
  if (tid != -1) {
    linked_list_node *node = find(tid);
    ((tinfo *)node->data)->time -= 1;
    if (((tinfo *)node->data)->time == 0) {
      level_down(tid);
      return 1;
    } else {
      return 0;
    }
  }
  return 1;
}

void scheduler_exit(int tid) {
  if (mlfq_scheduler.current == tid)
    mlfq_scheduler.current = -1;
}