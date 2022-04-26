#include "defs.h"
#include "fs.h"
#include "riscv.h"
#include "thread.h"

static processor cpu;

void init_cpu(thread idle, thread_pool pool) {
  cpu.idle = idle;
  cpu.pool = pool;
  cpu.occupied = 0;
}

void add_to_cpu(thread thread) { add_to_pool(&cpu.pool, thread); }

void exit_from_cpu(usize code) {
  disable_and_store();
  int tid = cpu.current.tid;
  exit_from_pool(&cpu.pool, tid);
  if (cpu.current.thread.waiting_tid != -1)
    wakeup_cpu(cpu.current.thread.waiting_tid);

  switch_thread(&cpu.current.thread, &cpu.idle);
}

void run_cpu() {
  // from boot thread to idle and never switch back
  thread boot = {0L, 0L};
  switch_thread(&boot, &cpu.idle);
}

void idle_main() {
  disable_and_store();
  while (1) {
    running_thread rt = acquire_from_pool(&cpu.pool);
    if (rt.tid != -1) {
      cpu.current = rt;
      cpu.occupied = 1;
      switch_thread(&cpu.idle, &cpu.current.thread);
      cpu.occupied = 0;
      retrieve_to_pool(&cpu.pool, cpu.current);
    } else {
      enable_and_wfi();
      disable_and_store();
    }
  }
}

void tick_cpu() {
  if (!cpu.occupied)
    return;
  if (!tick_pool(&cpu.pool))
    return;
  // when runs out the time slice, switch back to idle
  usize flags = disable_and_store();
  switch_thread(&cpu.current.thread, &cpu.idle);

  // switch back to current thread and restore flags
  restore_sstatus(flags);
}

int get_current_tid() { return cpu.current.tid; }
thread *get_current_thread() { return &cpu.current.thread; }

void yield_cpu() {
  if (cpu.occupied) {
    usize flags = disable_and_store();
    int tid = cpu.current.tid;
    thread_info *ti = &cpu.pool.threads[tid];
    ti->status = SLEEPING;
    switch_thread(&cpu.current.thread, &cpu.idle);

    restore_sstatus(flags);
  }
}

void wakeup_cpu(int tid) {
  thread_info *ti = &cpu.pool.threads[tid];
  ti->status = READY;
  scheduler_push(tid);
}

int execute_cpu(char *path, int hostTid) {
  inode *res = lookup(0, path);
  if (res == 0) {
    printf("Command not found!\n");
    return 0;
  }
  char *buf = kalloc(res->size);
  read_all(res, buf);
  thread t = new_uthread(buf);
  t.waiting_tid = hostTid;
  kfree(buf);
  add_to_cpu(t);
  return 1;
}