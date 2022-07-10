#ifndef _SBI_H_
#define _SBI_H_

// sbi call no.
#define SBI_SET_TIMER 0x0
#define SBI_CONSOLE_PUTCHAR 0x1
#define SBI_CONSOLE_GETCHAR 0x2
#define SBI_CLEAR_IPI 0x3
#define SBI_SEND_IPI 0x4
#define SBI_REMOTE_FENCE_I 0x5
#define SBI_REMOTE_SFENCE_VMA 0x6
#define SBI_REMOTE_SFENCE_VMA_ASID 0x7
#define SBI_SHUTDOWN 0x8

#define SBI_EID_HSM 0x48534D
#define SBI_FID_HSM_START 0
#define SBI_FID_HSM_STOP 1
#define SBI_FID_HSM_STATUS 2

// use ecall for sbi call
#define SBI_ECALL(__num, __a0, __a1, __a2)                                     \
  ({                                                                           \
    register unsigned long a0 asm("a0") = (unsigned long)(__a0);               \
    register unsigned long a1 asm("a1") = (unsigned long)(__a1);               \
    register unsigned long a2 asm("a2") = (unsigned long)(__a2);               \
    register unsigned long a7 asm("a7") = (unsigned long)(__num);              \
    asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a7) : "memory");   \
    a0;                                                                        \
  })

#define SBI_ECALL_0(__num) SBI_ECALL(__num, 0, 0, 0)
#define SBI_ECALL_1(__num, __a0) SBI_ECALL(__num, __a0, 0, 0)
#define SBI_ECALL_2(__num, __a0, __a1) SBI_ECALL(__num, __a0, __a1, 0)

#define __cmpxchg(ptr, old, new, size)                                         \
  ({                                                                           \
    __typeof__(ptr) __ptr = (ptr);                                             \
    __typeof__(*(ptr)) __old = (old);                                          \
    __typeof__(*(ptr)) __new = (new);                                          \
    __typeof__(*(ptr)) __ret;                                                  \
    register unsigned int __rc;                                                \
    switch (size) {                                                            \
    case 4:                                                                    \
      __asm__ __volatile__("0: lr.w %0, %2\n"                                  \
                           "   bne  %0, %z3, 1f\n"                             \
                           "   sc.w.rl %1, %z4, %2\n"                          \
                           "   bnez %1, 0b\n"                                  \
                           "   fence rw, rw\n"                                 \
                           "1:\n"                                              \
                           : "=&r"(__ret), "=&r"(__rc), "+A"(*__ptr)           \
                           : "rJ"((long)__old), "rJ"(__new)                    \
                           : "memory");                                        \
      break;                                                                   \
    case 8:                                                                    \
      __asm__ __volatile__("0: lr.d %0, %2\n"                                  \
                           "   bne %0, %z3, 1f\n"                              \
                           "   sc.d.rl %1, %z4, %2\n"                          \
                           "   bnez %1, 0b\n"                                  \
                           "   fence rw, rw\n"                                 \
                           "1:\n"                                              \
                           : "=&r"(__ret), "=&r"(__rc), "+A"(*__ptr)           \
                           : "rJ"(__old), "rJ"(__new)                          \
                           : "memory");                                        \
      break;                                                                   \
    }                                                                          \
    __ret;                                                                     \
  })

typedef struct {
  usize error;
  usize value;
} sbi_result;

sbi_result sbi_call(usize eid, usize fid, usize arg0, usize arg1, usize arg2);
sbi_result sbi_hart_start(usize hartid, usize start_addr, usize opaque);

#endif