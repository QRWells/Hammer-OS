#ifndef _DEFS_H_
#define _DEFS_H_

#include "types.h"
#include "sbi.h"
#include "interrupt.h"
#include "mapping.h"
#include "thread.h"

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
void handle_syscall(interrupt_context *context);

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
void map_framed_segment(mapping m, segment segment);
mapping new_kernel_mapping();
void map_framed_and_copy(mapping m, segment segment, char *data, usize length);

// thread.c
void init_thread();
void switch_thread(thread *self, thread *target);

// threadpool.c
thread_pool new_thread_pool(scheduler scheduler);
void add_to_pool(thread_pool *pool, thread thread);
running_thread acquire_from_pool(thread_pool *pool);
void retrieve_to_pool(thread_pool *pool, running_thread rt);
int tick_pool(thread_pool *pool);
void exit_from_pool(thread_pool *pool, int tid);

// processor.c
void tick_cpu();
void idle_main();
void init_cpu(thread idle, thread_pool pool);
void add_to_cpu(thread thread);
void exit_from_cpu(usize code);
void run_cpu();

// scheduler interface
void scheduler_init();
void scheduler_push(int tid);
int scheduler_pop();
int scheduler_tick();
void scheduler_exit(int tid);

// elf.c
mapping new_user_mapping(char *elf);
usize convert_elf_flags(u32 flags);

// syscall.c
usize syscall(usize id, usize args[3], interrupt_context *context);

// fs.c
void init_fs();

// string.c
int strcmp(char *s1, char *s2);
int strlen(char *s);

#endif // _DEFS_H_