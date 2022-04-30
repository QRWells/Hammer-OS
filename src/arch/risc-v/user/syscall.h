#ifndef _SYSCALL_H_
#define _SYSCALL_H_

typedef enum {
  SHUTDOWN = 13,
  LSDIR = 20,
  CDDIR = 21,
  PWD = 22,
  OPEN = 56,
  CLOSE = 57,
  READ = 63,
  WRITE = 64,
  EXIT = 93,
  EXEC = 221,
} syscall_id;

#define sys_call(__num, __a0, __a1, __a2, __a3)                                \
  ({                                                                           \
    register unsigned long a0 asm("a0") = (unsigned long)(__a0);               \
    register unsigned long a1 asm("a1") = (unsigned long)(__a1);               \
    register unsigned long a2 asm("a2") = (unsigned long)(__a2);               \
    register unsigned long a3 asm("a3") = (unsigned long)(__a3);               \
    register unsigned long a7 asm("a7") = (unsigned long)(__num);              \
    asm volatile("ecall"                                                       \
                 : "+r"(a0)                                                    \
                 : "r"(a1), "r"(a2), "r"(a3), "r"(a7)                          \
                 : "memory");                                                  \
    a0;                                                                        \
  })

#define sys_shut() sys_call(SHUTDOWN, 0, 0, 0, 0)
#define sys_lsdir(__a0, __a1) sys_call(LSDIR, __a0, __a1, 0, 0)
#define sys_cddir(__a0, __a1) sys_call(CDDIR, __a0, __a1, 0, 0)
#define sys_pwd(__a0) sys_call(PWD, __a0, 0, 0, 0)
#define sys_open(__a0) sys_call(OPEN, __a0, 0, 0, 0)
#define sys_close(__a0) sys_call(CLOSE, __a0, 0, 0, 0)
#define sys_read(__a0, __a1, __a2) sys_call(READ, __a0, __a1, __a2, 0)
#define sys_write(__a0) sys_call(WRITE, __a0, 0, 0, 0)
#define sys_exit(__a0) sys_call(EXIT, __a0, 0, 0, 0)
#define sys_exec(__a0, __a1) sys_call(EXEC, __a0, __a1, 0, 0)

#ifdef __INTELLISENSE__
#pragma diag_suppress 1118
#endif

#endif // _SYSCALL_H_