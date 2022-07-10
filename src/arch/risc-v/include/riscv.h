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

#define SSTATUS_SUM (1L << 18)
#define SSTATUS_SPP (1L << 8)
#define SSTATUS_SPIE (1L << 5)
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

static inline u64 r_satp() {
  u64 x;
  asm volatile("csrr %0, satp" : "=r"(x));
  return x;
}

static inline void w_satp(u64 x) { asm volatile("csrw satp, %0" : : "r"(x)); }

// enable asynchroneous interrupt, wait for interrupt
static inline void enable_and_wfi() {
  asm volatile("csrsi sstatus, 1 << 1; wfi");
}

// disable asynchroneous interrupt and save sstatus
static inline usize disable_and_store() {
  usize x;
  asm volatile("csrrci %0, sstatus, 1 << 1" : "=r"(x));
  return x;
}

// restore sstatus
static inline void restore_sstatus(usize flags) {
  asm volatile("csrs sstatus, %0" ::"r"(flags));
}

static inline u64 r_fp() {
  u64 x;
  asm volatile("mv %0, s0" : "=r"(x));
  return x;
}

#endif // _RISCV_H_