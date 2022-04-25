#include "../include/elf.h"
#include "thread.h"
#include "constants.h"
#include "defs.h"
#include "fs.h"
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

usize push_context_to_stack(thread_context self, usize stack_top) {
  thread_context *ptr = (thread_context *)(stack_top - sizeof(thread_context));
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
  printf("kthread entry at: %p\n", entry);
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
  scheduler s = {scheduler_init, scheduler_push, scheduler_pop, scheduler_tick,
                 scheduler_exit};
  s.init();
  thread_pool pool = new_thread_pool(s);
  thread idle = new_kthread((usize)idle_main);
  init_cpu(idle, pool);

  for (usize i = 0; i < 5; i++) {
    thread t = new_kthread((usize)test_thread);
    usize args[8];
    args[0] = i;
    append_arguments(&t, args);
    add_to_cpu(t);
  }

  // inode *test_inode = lookup(0, "/bin/test");
  // char *buf = kalloc(test_inode->size);
  // read_all(test_inode, buf);
  // thread t = new_uthread(buf);
  // kfree(buf);

  // add_to_cpu(t);

  printf("***** Init Thread *****\n");
}

usize new_uthread_context(usize entry, usize ustack_top, usize kstack_top,
                          usize satp) {
  interrupt_context ic;
  ic.x[2] = ustack_top;
  ic.sepc = entry;
  ic.sstatus = r_sstatus();
  // set to U-Mode
  ic.sstatus &= ~SSTATUS_SPP;
  // enable async interrupts
  ic.sstatus |= SSTATUS_SPIE;
  ic.sstatus &= ~SSTATUS_SIE;
  thread_context tc;
  extern void __restore();
  tc.ra = (usize)__restore;
  tc.satp = satp;
  tc.ic = ic;
  return push_context_to_stack(tc, kstack_top);
}

thread new_uthread(char *data) {
  mapping m = new_user_mapping(data);
  usize ustack_bottom = USER_STACK_OFFSET,
        ustack_top = USER_STACK_OFFSET + USER_STACK_SIZE;
  // map user stack
  segment s = {ustack_bottom, ustack_top,
               1L | PTE_USER | PTE_READABLE | PTE_WRITABLE};
  map_framed_segment(m, s);

  usize kstack = new_kstack();
  usize entry_addr = ((elf_header *)data)->entry;
  printf("uthread entry at: %p\n", entry_addr);
  process p = {m.root_ppn | (1L << 63)};
  usize context = new_uthread_context(entry_addr, ustack_top,
                                      kstack + KERNEL_STACK_SIZE, p.satp);
  thread t = {context, kstack, p};
  return t;
}