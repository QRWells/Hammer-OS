    .section .text.entry
    .globl _start
    # set sp and jump to kmain
_start:
    la sp, bootstacktop
    
    call main
    
    .section .bss.stack
    .align 12
    .global bootstack
bootstack:
    # 64KB boot stack
    .space 1024 * 64
    .global bootstacktop
bootstacktop: