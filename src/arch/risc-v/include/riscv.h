#ifndef _RISCV_H_
#define _RISCV_H_

#include "types.h"

static inline usize r_scause() {
  usize x;
  asm volatile("csrr %0, scause" : "=r"(x));
  return x;
}

static inline usize r_sepc() {
  usize x;
  asm volatile("csrr %0, sepc" : "=r"(x));
  return x;
}

#define MODE_DIRECT 0x0
#define MODE_VECTOR 0x1
// stvec, set interrupt procedure and mode
static inline void w_stvec(usize x) {
  asm volatile("csrw stvec, %0" : : "r"(x));
}

#define SIE_SEIE (1L << 9) // external interrupt
#define SIE_STIE (1L << 5) // clock interrupt
#define SIE_SSIE (1L << 1) // software interrupt

static inline usize r_sie() {
  usize x;
  asm volatile("csrr %0, sie" : "=r"(x));
  return x;
}

static inline void w_sie(usize x) { asm volatile("csrw sie, %0" : : "r"(x)); }

#define SSTATUS_SIE (1L << 1) // Supervisor mode Interrupt Enable
#define SSTATUS_UIE (1L << 0) // User mode Interrupt Enable

// Supervisor Status Register
static inline usize r_sstatus() {
  usize x;
  asm volatile("csrr %0, sstatus" : "=r"(x));
  return x;
}

static inline void w_sstatus(usize x) {
  asm volatile("csrw sstatus, %0" : : "r"(x));
}

// read hardware clock
static inline usize r_time() {
  usize x;
  asm volatile("csrr %0, time" : "=r"(x));
  return x;
}

#endif // _RISCV_H_