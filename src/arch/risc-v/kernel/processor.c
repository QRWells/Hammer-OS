#include "thread.h"
#include "defs.h"
#include "riscv.h"

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
  printf("Thread %d exited, exit code = %d\n", tid, code);
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
      printf("\n>>>> will switch_to thread %d in idle_main!\n",
             cpu.current.tid);
      switch_thread(&cpu.idle, &cpu.current.thread);

      printf("<<<< switch_back to idle in idle_main!\n");
      cpu.occupied = 0;
      retrieve_to_pool(&cpu.pool, cpu.current);
    } else {
      enable_and_wfi();
      disable_and_store();
    }
  }
}

void tick_cpu() {
  if (cpu.occupied) {
    if (tick_pool(&cpu.pool)) {
      // when runs out the time slice, switch back to idle
      usize flags = disable_and_store();
      switch_thread(&cpu.current.thread, &cpu.idle);

      // switch back to current thread and restore flags
      restore_sstatus(flags);
    }
  }
}