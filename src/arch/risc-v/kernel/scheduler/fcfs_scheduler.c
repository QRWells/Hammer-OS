#include "types.h"
#include "constants.h"
#include "defs.h"

typedef struct Node {
  int tid;
} tinfo;

struct {
  tinfo threads[MAX_THREAD];
  int front;
  int rear;
  int size;
  int current;
} fcfs_scheduler;

void scheduler_init() {
  fcfs_scheduler.current = -1;
  fcfs_scheduler.front = -1;
  fcfs_scheduler.rear = -1;
  fcfs_scheduler.size = 0;
}

void scheduler_push(int tid) {
  if (fcfs_scheduler.size >= MAX_THREAD)
    panic("queue is full!\n");

  tinfo t = {tid};
  fcfs_scheduler.rear++;
  fcfs_scheduler.rear %= MAX_THREAD;
  fcfs_scheduler.size++;
  fcfs_scheduler.threads[fcfs_scheduler.rear] = t;
}

int scheduler_pop() {
  if (fcfs_scheduler.size == 0)
    return -1;

  fcfs_scheduler.front++;
  fcfs_scheduler.front %= MAX_THREAD;
  fcfs_scheduler.size--;
  int tid = fcfs_scheduler.threads[fcfs_scheduler.front].tid;
  fcfs_scheduler.current = tid;
  return tid;
}

int scheduler_tick() { return 0; }

void scheduler_exit(int tid) {
  if (fcfs_scheduler.current == tid)
    fcfs_scheduler.current = -1;
}