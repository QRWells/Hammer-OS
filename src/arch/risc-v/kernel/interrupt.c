#include "interrupt.h"
#include "defs.h"
#include "riscv.h"
#include "stdin.h"
#include "types.h"

asm(".include \"interrupt.asm\"");
// asm(".include \"trap.S\"");

// enable interrupts directly by write kernel memory
void init_external_interrupt() {
  *(u32 *)(0x0C000004 + KERNEL_MAP_OFFSET) = 0x1U;
  *(u32 *)(0x0C000028 + KERNEL_MAP_OFFSET) = 0x1U;
}

void init_serial_interrupt() {
  *(u8 *)(0x10000001 + KERNEL_MAP_OFFSET) = 0x01U;
  *(u8 *)(0x10000004 + KERNEL_MAP_OFFSET) = 0x0bU;
}

void init_interrupt() {
  extern void __interrupt();
  int cpuid = r_tp();

  w_sie(r_sie() | SIE_SEIE | SIE_SSIE);

  if (cpuid == 0) {
    init_external_interrupt();
    init_serial_interrupt();
  }

  *(u32 *)(0x0C002080 + (cpuid * 0x100) + KERNEL_MAP_OFFSET) = 0x402U;
  *(u32 *)(0x0C201000 + (cpuid * 0x2000) + KERNEL_MAP_OFFSET) = 0x0U;

  asm volatile("csrw sscratch, 0");
  // Set the exception vector address
  w_stvec((usize)__interrupt);

  printf("init_interrupt: cpuid = %d\n", cpuid);
}

void handle_interrupt(interrupt_context *context, usize scause, usize stval) {
  switch (scause) {
    case BREAKPOINT:
      breakpoint(context);
      break;
    case SUPERVISOR_TIMER:
      supervisor_timer();
      break;
    case USER_ENV_CALL:
      handle_syscall(context);
      break;
    case SUPERVISOR_EXTERNAL:
      supervisor_external();
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

void supervisor_external() {
  usize ret = console_getchar();
  if (ret != -1) {
    char ch = (char)ret;
    if (ch == '\r') {
      push_char('\n');
    } else {
      push_char(ch);
    }
  }
}

void fault(interrupt_context *context, usize scause, usize stval) {
  printf(
      "Unhandled interrupt from hart %d!\nscause\t= %p\nsepc\t= "
      "%p\nstval\t= %p\n",
      context->x[4], scause, context->sepc, stval);
  // backtrace();
  panic("");
}

void panic(char *s) {
  printf("panic: ");
  printf(s);
  printf("\n");
  shutdown();
}

void handle_syscall(interrupt_context *context) {
  context->sepc += 4;
  usize ret = syscall(context->x[17],
                      (usize[]){context->x[10], context->x[11], context->x[12]},
                      context);
  context->x[10] = ret;
}