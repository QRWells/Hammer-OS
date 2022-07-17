#include "defs.h"
#include "fs.h"
#include "riscv.h"
#include "thread.h"

static processor cpu[MAX_CPU];

void init_cpu(thread idle, thread_pool pool) {
  usize cpuid = r_tp();
  cpu[cpuid].idle = idle;
  cpu[cpuid].pool = pool;
  cpu[cpuid].occupied = 0;
}

void add_to_cpu(thread thread) {
  usize cpuid = r_tp();
  thread.arrive_time = r_time();
  add_to_pool(&cpu[cpuid].pool, thread);
}

void exit_from_cpu(usize code) {
  disable_and_store();
  usize cpuid = r_tp();
  int tid = cpu[cpuid].current.tid;
  cpu[cpuid].current.thread.end_time = r_time();
  printf("Thread %d arrived at %d\n", tid,
         cpu[cpuid].current.thread.arrive_time);
  printf("Thread %d started at %d\n", tid,
         cpu[cpuid].current.thread.start_time);
  printf("Thread %d exited at %d\n", tid, cpu[cpuid].current.thread.end_time);
  exit_from_pool(&cpu[cpuid].pool, tid);
  if (cpu[cpuid].current.thread.waiting_tid != -1)
    wakeup_cpu(cpu[cpuid].current.thread.waiting_tid);

  switch_thread(&cpu[cpuid].current.thread, &cpu[cpuid].idle);
}

void run_cpu() {
  // from boot thread to idle and never switch back
  usize cpuid = r_tp();
  thread boot;
  boot.context_addr = 0;
  boot.kstack = 0;
  boot.waiting_tid = -1;
  switch_thread(&boot, &cpu[cpuid].idle);
}

void idle_main() {
  disable_and_store();
  usize cpuid = r_tp();
  while (1) {
    running_thread rt = acquire_from_pool(&cpu[cpuid].pool);
    if (rt.tid != -1) {
      cpu[cpuid].current = rt;
      cpu[cpuid].occupied = 1;
      switch_thread(&cpu[cpuid].idle, &cpu[cpuid].current.thread);
      cpu[cpuid].occupied = 0;
      retrieve_to_pool(&cpu[cpuid].pool, cpu[cpuid].current);
    } else {
      enable_and_wfi();
      disable_and_store();
    }
  }
}

void tick_cpu() {
  usize cpuid = r_tp();
  if (!cpu[cpuid].occupied) return;
  if (!tick_pool(&cpu[cpuid].pool)) return;
  // when runs out the time slice, switch back to idle
  usize flags = disable_and_store();
  switch_thread(&cpu[cpuid].current.thread, &cpu[cpuid].idle);

  // switch back to current thread and restore flags
  restore_sstatus(flags);
}

int get_current_tid() {
  usize cpuid = r_tp();
  return cpu[cpuid].current.tid;
}
thread *get_current_thread() {
  usize cpuid = r_tp();
  return &cpu[cpuid].current.thread;
}

void yield_cpu() {
  usize cpuid = r_tp();
  if (cpu[cpuid].occupied) {
    usize flags = disable_and_store();
    int tid = cpu[cpuid].current.tid;
    thread_info *ti = &cpu[cpuid].pool.threads[tid];
    ti->status = SLEEPING;
    switch_thread(&cpu[cpuid].current.thread, &cpu[cpuid].idle);

    restore_sstatus(flags);
  }
}

void wakeup_cpu(int tid) {
  usize cpuid = r_tp();
  thread_info *ti = &cpu[cpuid].pool.threads[tid];
  ti->status = READY;
  scheduler_push(tid);
}

int execute_cpu(inode *node, int hostTid) {
  if (node->type == TYPE_DIR) {
    printf("%s: is a directory!\n", node->filename);
    return 0;
  }
  char *buf = kalloc(node->size);
  read_all(node, buf);
  thread t = new_uthread(buf);
  t.waiting_tid = hostTid;
  kfree(buf);
  add_to_cpu(t);
  return 1;
}