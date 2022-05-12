#include "types.h"
#include "constants.h"
#include "defs.h"
#include "linked_list.h"

#define LEVEL 3
#define MLFQ_PROMOTION_THRESHOLD 2

struct {
  int time_used[MAX_THREAD];
  int occupied[MAX_THREAD];
  linked_list queues[LEVEL];
  int current;
  int max_time[LEVEL - 1];
  int promotion_time;
} mlfq_scheduler;

void promote_all() {
  for (int i = 1; i < LEVEL; i++) {
    linked_list_node *node =
        linked_list_remove_first(mlfq_scheduler.queues + i);
    while (node != NULL) {
      int t = node->data;
      mlfq_scheduler.time_used[t] = mlfq_scheduler.max_time[0];
      linked_list_add_last(mlfq_scheduler.queues, node);
      node = linked_list_remove_first(mlfq_scheduler.queues + i);
    }
  }
}

int get_level(int tid) {
  for (int i = 0; i < LEVEL; ++i) {
    linked_list_node *node = mlfq_scheduler.queues[i].first;
    while (node != NULL) {
      if ((int)node->data == tid)
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
      if ((int)node->data == tid)
        return node;
      node = node->next;
    }
  }
  return NULL;
}

void level_down(int tid) {
  int level = get_level(tid);
  if (level >= LEVEL - 1)
    return;
  linked_list_node *node = find(tid);
  linked_list_remove(&mlfq_scheduler.queues[level], node);
  linked_list_add_last(&mlfq_scheduler.queues[level + 1], node);
}

void scheduler_init() {
  for (int i = 0; i < LEVEL; i++)
    mlfq_scheduler.queues[i] = *linked_list_init();

  mlfq_scheduler.max_time[0] = 1;
  mlfq_scheduler.max_time[1] = 2;

  for (int i = 0; i < MAX_THREAD; i++) {
    mlfq_scheduler.time_used[i] = 0;
    mlfq_scheduler.occupied[i] = 0;
  }
  mlfq_scheduler.current = -1;
  mlfq_scheduler.promotion_time = MLFQ_PROMOTION_THRESHOLD; // magic number
}

void scheduler_push(int tid) {
  if (mlfq_scheduler.queues[0].size >= MAX_THREAD)
    panic("queue is full!\n");
  if (mlfq_scheduler.occupied[tid])
    return;

  mlfq_scheduler.occupied[tid] = 1;
  if (mlfq_scheduler.time_used[tid] == 0)
    mlfq_scheduler.time_used[tid] = mlfq_scheduler.max_time[0];
  linked_list_node *node = linked_list_create_node((u64)tid);
  linked_list_add_last(&mlfq_scheduler.queues[0], node);
}

int scheduler_pop() {
  linked_list_node *node = 0;
  for (int i = 0; i < LEVEL; i++) {
    if (mlfq_scheduler.queues[i].size > 0) {
      node = linked_list_remove_first(&mlfq_scheduler.queues[i]);
      linked_list_add_last(&mlfq_scheduler.queues[i], node);
      break;
    }
  }
  if (node == NULL)
    return -1;
  int tid = node->data;
  mlfq_scheduler.current = tid;
  return tid;
}

int scheduler_tick() {
  mlfq_scheduler.promotion_time--;
  if (mlfq_scheduler.promotion_time <= 0) {
    mlfq_scheduler.promotion_time = MLFQ_PROMOTION_THRESHOLD;
    promote_all();
  }
  int tid = mlfq_scheduler.current;
  int level = get_level(tid);
  if (tid != -1) {
    mlfq_scheduler.time_used[tid] -= 1;
    if (mlfq_scheduler.time_used[tid] > 0)
      return 0;
    else {
      level_down(tid);
      if (level < LEVEL - 1)
        mlfq_scheduler.time_used[tid] = mlfq_scheduler.max_time[level];
    }
  }
  return 1;
}

void scheduler_exit(int tid) {
  if (mlfq_scheduler.current == tid) {
    int level = get_level(tid);
    linked_list_node *node = find(tid);
    linked_list_remove(&mlfq_scheduler.queues[level], node);
    kfree(node);
    mlfq_scheduler.occupied[tid] = 0;
    mlfq_scheduler.current = -1;
  }
}