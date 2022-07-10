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

sbi_result sbi_call(usize eid, usize fid, usize arg0, usize arg1, usize arg2) {
  sbi_result res;
  register unsigned long a0 asm("a0") = (unsigned long)(arg0);
  register unsigned long a1 asm("a1") = (unsigned long)(arg1);
  register unsigned long a2 asm("a2") = (unsigned long)(arg2);
  register unsigned long a7 asm("a7") = (unsigned long)(fid);
  register unsigned long a6 asm("a6") = (unsigned long)(eid);
  asm volatile("ecall"
               : "+r"(a0), "+r"(a1)
               : "r"(a0), "r"(a1), "r"(a2), "r"(a7), "r"(a6)
               : "memory");
  res.error = a0;
  res.value = a1;
  return res;
}

sbi_result sbi_hart_start(usize hartid, usize start_addr, usize opaque) {
  return sbi_call(SBI_EID_HSM, SBI_FID_HSM_START, hartid, start_addr, opaque);
}