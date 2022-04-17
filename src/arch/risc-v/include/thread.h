#ifndef _THREAD_H_
#define _THREAD_H_

#include "types.h"
#include "constants.h"
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

typedef enum { READY, RUNNING, SLEEPING, EXITED } thread_state;

typedef struct {
  thread_state status;
  int tid;
  int occupied;
  thread thread;
} thread_info;

typedef struct {
  int tid;
  thread thread;
} running_thread;

typedef struct {
  void (*init)(void);
  void (*push)(int);
  // select a thread to run, if no thread is ready, return -1
  int (*pop)(void);
  // remind the scheduler that the current thread has run for one tick
  // return value indicates whether the current thread should be switched out
  int (*tick)(void);
  // terminate a thread
  void (*exit)(int);
} scheduler;

typedef struct {
  thread_info threads[MAX_THREAD];
  scheduler scheduler;
} thread_pool;

typedef struct {
  thread_pool pool;
  thread idle;
  running_thread current;
  int occupied;
} processor;

#endif // _THREAD_H_