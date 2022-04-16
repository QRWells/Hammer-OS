#include "defs.h"

void consolePutchar(u64 c) { SBI_ECALL_1(SBI_CONSOLE_PUTCHAR, c); }

u64 consoleGetchar() { return SBI_ECALL_0(SBI_CONSOLE_GETCHAR); }

void shutdown() {
  SBI_ECALL_0(SBI_SHUTDOWN);
  while (1)
    ;
}
