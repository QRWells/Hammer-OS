#include "thread.h"
#include "../include/elf.h"
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

usize push_context_to_stack(thread_context tc, interrupt_context ic,
                            usize stack_top) {
  interrupt_context *ptr1 =
      (interrupt_context *)(stack_top - sizeof(interrupt_context));
  *ptr1 = ic;

  thread_context *ptr2 =
      (thread_context *)((usize)ptr1 - sizeof(thread_context));
  *ptr2 = tc;

  return (usize)ptr2;
}

usize new_kthread_context(usize entry, usize kstack_top, usize satp) {
  interrupt_context ic;
  ic.x[2] = kstack_top;
  ic.x[4] = r_tp();
  ic.sepc = entry;
  ic.sstatus = r_sstatus();
  ic.sstatus |= SSTATUS_SPP;
  ic.sstatus |= SSTATUS_SPIE;
  ic.sstatus &= ~SSTATUS_SIE;
  thread_context tc;
  extern void __restore();
  tc.ra = (usize)__restore;
  tc.satp = satp;
  return push_context_to_stack(tc, ic, kstack_top);
}

thread new_kthread(usize entry) {
  usize stackBottom = new_kstack();
  process p;
  p.satp = r_satp();
  for (int i = 0; i < 3; i++) p.fd_occupied[i] = 1;
  usize contextAddr =
      new_kthread_context(entry, stackBottom + KERNEL_STACK_SIZE, p.satp);
  printf("kthread entry at: %p\n", entry);
  thread t = {contextAddr, stackBottom, p, -1, 0, 0, 0};
  return t;
}

thread new_boot_thread() {
  thread t;
  t.context_addr = 0L;
  t.kstack = 0L;
  t.waiting_tid = -1;
  return t;
}

void test_thread(usize arg) {
  printf("Begin of thread %d\n", arg);
  int i;
  for (i = 0; i < 1000; i++) printf("%d", arg);

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

  inode *test_inode = lookup(0, "/bin/sh");
  char *buf = kalloc(test_inode->size);
  read_all(test_inode, buf);
  thread t = new_uthread(buf);
  kfree(buf);

  add_to_cpu(t);

  printf("***** Init Thread *****\n");
}

void init_thread_other() {
  scheduler s = {scheduler_init, scheduler_push, scheduler_pop, scheduler_tick,
                 scheduler_exit};
  s.init();
  thread_pool pool = new_thread_pool(s);
  thread idle = new_kthread((usize)idle_main);
  init_cpu(idle, pool);
}

usize new_uthread_context(usize entry, usize ustack_top, usize kstack_top,
                          usize satp) {
  interrupt_context ic;
  ic.x[2] = ustack_top;
  ic.x[4] = r_tp();
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
  return push_context_to_stack(tc, ic, kstack_top);
}

thread new_uthread(char *data) {
  mapping m = new_user_mapping(data);
  usize ustack_bottom = USER_STACK_OFFSET;
  usize ustack_top = USER_STACK_OFFSET + USER_STACK_SIZE;
  // map user stack
  segment s = {ustack_bottom, ustack_top,
               PTE_VALID | PTE_USER | PTE_READABLE | PTE_WRITABLE};
  map_framed_segment(m, s);

  usize kstack = new_kstack();
  usize entry_addr = ((elf_header *)data)->entry;
  // printf("uthread entry at: %p\n", entry_addr);
  process p;
  p.satp = m.root_ppn | (1L << 63);
  for (int i = 0; i < 3; i++) p.fd_occupied[i] = 1;
  usize context = new_uthread_context(entry_addr, ustack_top,
                                      kstack + KERNEL_STACK_SIZE, p.satp);
  thread t = {context, kstack, p, -1, 0, 0, 0};
  return t;
}