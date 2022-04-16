#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "interrupt.h"

asm(".include \"interrupt.asm\"");

void handle_interrupt(interrupt_context *context, usize scause, usize stval) {
  switch (scause) {
  case 3L:
    breakpoint(context);
    break;
  default:
    printf("Unhandled interrupt!\nscause\t= %p\nsepc\t= %p\nstval\t= %p\n",
           scause, context->sepc, stval);
    panic("");
  }
}

void init_interrupt() {
  extern void __interrupt();
  w_stvec((usize)__interrupt | MODE_DIRECT);
  printf("***** Init Interrupt *****\n");
}

void breakpoint(interrupt_context *context) {
  printf("Breakpoint at %p\n", context->sepc);
  // ebreak is 2 bytes long
  context->sepc += 2;
}

void panic(char *s) {
  printf("panic: ");
  printf(s);
  printf("\n");
  shutdown();
}