#include "defs.h"
#include "thread.h"
#include "constants.h"
#include "riscv.h"

__attribute__((naked, noinline)) void switch_context(usize *self,
                                                     usize *target) {
  asm volatile(".include \"switch.asm\"");
}

void switch_thread(thread *self, thread *target) {
  switch_context(&self->context_addr, &target->context_addr);
}

usize new_kstack() {
  usize bottom = (usize)kalloc(KERNEL_STACK_SIZE);
  return bottom;
}

usize push_context_to_stack(thread_context self, usize stackTop) {
  thread_context *ptr = (thread_context *)(stackTop - sizeof(thread_context));
  *ptr = self;
  return (usize)ptr;
}

usize new_kthread_context(usize entry, usize kstack_top, usize satp) {
  interrupt_context ic;
  ic.x[2] = kstack_top;
  ic.sepc = entry;
  ic.sstatus = r_sstatus();
  ic.sstatus |= SSTATUS_SPP;
  ic.sstatus |= SSTATUS_SPIE;
  ic.sstatus &= ~SSTATUS_SIE;
  thread_context tc;
  extern void __restore();
  tc.ra = (usize)__restore;
  tc.satp = satp;
  tc.ic = ic;
  return push_context_to_stack(tc, kstack_top);
}

void append_arguments(thread *thread, usize args[8]) {
  thread_context *ptr = (thread_context *)thread->context_addr;
  ptr->ic.x[10] = args[0];
  ptr->ic.x[11] = args[1];
  ptr->ic.x[12] = args[2];
  ptr->ic.x[13] = args[3];
  ptr->ic.x[14] = args[4];
  ptr->ic.x[15] = args[5];
  ptr->ic.x[16] = args[6];
  ptr->ic.x[17] = args[7];
}

thread new_kthread(usize entry) {
  usize stackBottom = new_kstack();
  usize contextAddr =
      new_kthread_context(entry, stackBottom + KERNEL_STACK_SIZE, r_satp());
  thread t = {contextAddr, stackBottom};
  return t;
}

void temp_thread_func(thread *from, thread *current, usize c) {
  printf("The char passed by is ");
  console_putchar(c);
  console_putchar('\n');
  printf("Hello world from tempThread!\n");
  switch_thread(current, from);
}

thread new_boot_thread() {
  thread t = {0L, 0L};
  return t;
}

void init_thread() {
  thread bootThread = new_boot_thread();
  thread tempThread = new_kthread((usize)temp_thread_func);
  usize args[8];
  args[0] = (usize)&bootThread;
  args[1] = (usize)&tempThread;
  args[2] = (long)'M';
  append_arguments(&tempThread, args);
  switch_thread(&bootThread, &tempThread);
  printf("I'm back from tempThread!\n");
}

// ===================thread pool=====================

int alloc_tid(thread_pool *pool) {
  int i;
  for (i = 0; i < MAX_THREAD; i++) {
    // allocate an unused space, return tid
    if (!pool->threads[i].occupied) {
      return i;
    }
  }
  panic("Alloc tid failed!\n");
  return -1;
}

void add_to_pool(thread_pool *pool, thread thread) {
  int tid = allocTid(pool);
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