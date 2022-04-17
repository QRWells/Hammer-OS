#ifndef _THREAD_H_
#define _THREAD_H_

#include "types.h"
#include "interrupt.h"

typedef struct {
  usize ra;    // return address register
  usize satp;  // satp register
  usize s[12]; // caller saved registers
  interrupt_context ic;
} thread_context;

typedef struct {
  usize context_addr; // address of the thread's context
  usize kstack;       // bottom of stack
} thread;

#endif // _THREAD_H_