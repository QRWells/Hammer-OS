#include "types.h"
#include "constants.h"
#include "defs.h"

typedef struct {
  int valid;
  usize time;
  int prev;
  int next;
} rr_info;

struct {
  rr_info threads[MAX_THREAD + 1];
  usize max_time;
  int current;
} rr_scheduler;

void scheduler_init() {
  rr_scheduler.max_time = 1;
  rr_scheduler.current = 0;
  rr_info ri = {0, 0L, 0, 0};
  rr_scheduler.threads[0] = ri;
}

void scheduler_push(int tid) {
  tid += 1;
  if (tid + 1 > MAX_THREAD + 1)
    panic("Cannot push to scheduler!\n");

  if (rr_scheduler.threads[tid].time == 0)
    rr_scheduler.threads[tid].time = rr_scheduler.max_time;

  int prev = rr_scheduler.threads[0].prev;
  rr_scheduler.threads[tid].valid = 1;
  rr_scheduler.threads[prev].next = tid;
  rr_scheduler.threads[tid].prev = prev;
  rr_scheduler.threads[0].prev = tid;
  rr_scheduler.threads[tid].next = 0;
}

int scheduler_pop() {
  int ret = rr_scheduler.threads[0].next;
  if (ret == 0)
    return ret - 1;

  int next = rr_scheduler.threads[ret].next;
  int prev = rr_scheduler.threads[ret].prev;
  rr_scheduler.threads[next].prev = prev;
  rr_scheduler.threads[prev].next = next;
  rr_scheduler.threads[ret].prev = 0;
  rr_scheduler.threads[ret].next = 0;
  rr_scheduler.threads[ret].valid = 0;
  rr_scheduler.current = ret;

  return ret - 1;
}

int scheduler_tick() {
  int tid = rr_scheduler.current;
  if (tid != 0) {
    rr_scheduler.threads[tid].time -= 1;
    return rr_scheduler.threads[tid].time == 0 ? 1 : 0;
  }
  return 1;
}

void scheduler_exit(int tid) {
  tid += 1;
  if (rr_scheduler.current == tid)
    rr_scheduler.current = 0;
}