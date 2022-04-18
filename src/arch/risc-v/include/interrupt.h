#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_
#include "types.h"

// interrupt context, scause and stval as parameters, no need to save
typedef struct {
  usize x[32];
  usize sstatus;
  // interrupt return address
  usize sepc;
} interrupt_context;

/* RV64 中断发生时，机器根据中断类型自动设置 scause 寄存器 */
#define BREAKPOINT 3L                       /* 断点中断 */
#define USER_ENV_CALL 8L                    /* 来自 U-Mode 的系统调用 */
#define SUPERVISOR_TIMER 5L | (1L << 63)    /* S-Mode 的时钟中断 */
#define SUPERVISOR_EXTERNAL 9L | (1L << 63) /* S-Mode 的外部中断 */

#endif // _INTERRUPT_H_