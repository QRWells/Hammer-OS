#include "defs.h"
#include "thread.h"

thread_pool new_thread_pool(scheduler scheduler) {
  thread_pool pool;
  pool.scheduler = scheduler;
  return pool;
}

int alloc_tid(thread_pool *pool) {
  for (int i = 0; i < MAX_THREAD; i++)
    // allocate an unused space, return tid
    if (!pool->threads[i].occupied)
      return i;

  panic("Alloc tid failed!\n");
  return -1;
}

void add_to_pool(thread_pool *pool, thread thread) {
  int tid = alloc_tid(pool);
  pool->threads[tid].status = (thread_state)READY;
  pool->threads[tid].occupied = 1;
  pool->threads[tid].thread = thread;
  pool->scheduler.push(tid);
}

// retrieve an available thread from pool, return -1 if no available thread.
running_thread acquire_from_pool(thread_pool *pool) {
  int tid = pool->scheduler.pop();
  running_thread rt;
  rt.tid = tid;
  if (tid == -1)
    return rt;

  thread_info *ti = &pool->threads[tid];
  ti->status = (thread_state)RUNNING;
  ti->tid = tid;
  rt.thread = ti->thread;

  return rt;
}

// called when a thread is stopped and switch into shcedule thread.
void retrieve_to_pool(thread_pool *pool, running_thread rt) {
  int tid = rt.tid;
  if (!pool->threads[tid].occupied) {
    // this thread has exited, free its space.
    kfree((void *)pool->threads[tid].thread.kstack);
    return;
  }
  thread_info *ti = &pool->threads[tid];
  ti->thread = rt.thread;
  if (ti->status == (thread_state)RUNNING) {
    ti->status = (thread_state)READY;
    pool->scheduler.push(tid);
  }
}

int tick_pool(thread_pool *pool) { return pool->scheduler.tick(); }

// free the position of the tid
void exit_from_pool(thread_pool *pool, int tid) {
  pool->threads[tid].occupied = 0;
  pool->scheduler.exit(tid);
}