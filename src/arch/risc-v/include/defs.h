#ifndef _DEFS_H_
#define _DEFS_H_

#include "types.h"
#include "sbi.h"
#include "interrupt.h"
#include "mapping.h"

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
usize alloc_frame();

// mapping.c
void map_kernel();

// thread.c
void init_thread();
void switch_thread(thread *self, thread *target);
void add_to_pool(thread_pool *pool, thread thread);
running_thread acquire_from_pool(thread_pool *pool);
void retrieve_to_pool(thread_pool *pool, running_thread rt);
int tick_pool(thread_pool *pool);
void exit_from_pool(thread_pool *pool, int tid);

// processor.c
void tick_cpu();

#endif // _DEFS_H_