#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "interrupt.h"

asm(".include \"interrupt.asm\"");

void init_interrupt() {
  extern void __interrupt();
  w_stvec((usize)__interrupt | MODE_DIRECT);
  printf("***** Init Interrupt *****\n");
}

void handle_interrupt(interrupt_context *context, usize scause, usize stval) {
  switch (scause) {
  case 3L:
    breakpoint(context);
    break;
  case 5L | (1L << 63):
    supervisor_timer();
    break;
  default:
    fault(context, scause, stval);
    break;
  }
}

void breakpoint(interrupt_context *context) {
  printf("Breakpoint at %p\n", context->sepc);
  // ebreak is 2 bytes long
  context->sepc += 2;
}

void supervisor_timer() {
  tick();
  tick_cpu();
}

void fault(interrupt_context *context, usize scause, usize stval) {
  printf("Unhandled interrupt!\nscause\t= %p\nsepc\t= %p\nstval\t= %p\n",
         scause, context->sepc, stval);
  panic("");
}

void panic(char *s) {
  printf("panic: ");
  printf(s);
  printf("\n");
  shutdown();
}