#ifndef _DEFS_H_
#define _DEFS_H_

#include "types.h"
#include "sbi.h"
#include "interrupt.h"

// sbi.c
void console_putchar(usize c);
usize console_getchar();
void set_timer(usize time);
void shutdown();

// printf.c
void printf(char *fmt, ...);

// interrupt.c
void init_interrupt();
void handle_interrupt(interrupt_context *context, usize scause, usize stval);
void breakpoint(interrupt_context *context);
void supervisor_timer();
void fault(interrupt_context *context, usize scause, usize stval);
void panic(char *s);

// timer.c
void setTimeout();
void init_timer();
void tick();

// heap.c
void init_heap();
void *kalloc(u32 size);
void kfree(void *p);

// memory.c
void init_memory();

#endif // _DEFS_H_