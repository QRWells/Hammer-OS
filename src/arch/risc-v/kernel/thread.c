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

thread new_boot_thread() {
  thread t = {0L, 0L};
  return t;
}

void test_thread(usize arg) {
  printf("Begin of thread %d\n", arg);
  int i;
  for (i = 0; i < 1000; i++)
    printf("%d", arg);

  printf("\nEnd of thread %d\n", arg);
  exit_from_cpu(0);
  while (1)
    ;
}

void init_thread() {
  // thread bootThread = new_boot_thread();
  // thread tempThread = new_kthread((usize)temp_thread_func);
  // usize args[8];
  // args[0] = (usize)&bootThread;
  // args[1] = (usize)&tempThread;
  // args[2] = (long)'M';
  // append_arguments(&tempThread, args);
  // switch_thread(&bootThread, &tempThread);
  // printf("I'm back from tempThread!\n");
  scheduler s = {scheduler_init, scheduler_push, scheduler_pop, scheduler_tick,
                 scheduler_exit};
  s.init();
  thread_pool pool = new_thread_pool(s);
  thread idle = new_kthread((usize)idle_main);
  init_cpu(idle, pool);
  usize i;
  for (i = 0; i < 5; i++) {
    thread t = new_kthread((usize)test_thread);
    usize args[8];
    args[0] = i;
    append_arguments(&t, args);
    add_to_cpu(t);
  }
  printf("***** Init Thread *****\n");
}