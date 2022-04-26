#include "defs.h"

#ifdef __INTELLISENSE__
#pragma diag_suppress 1118
#endif

void console_putchar(usize c) { SBI_ECALL_1(SBI_CONSOLE_PUTCHAR, c); }

usize console_getchar() { return SBI_ECALL_0(SBI_CONSOLE_GETCHAR); }

void shutdown() {
  SBI_ECALL_0(SBI_SHUTDOWN);
  while (1)
    ;
}

void set_timer(usize time) { SBI_ECALL_1(SBI_SET_TIMER, time); }