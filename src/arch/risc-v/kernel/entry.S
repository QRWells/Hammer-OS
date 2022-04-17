    .section .text.entry
    .globl _start
    # set sp and jump to kmain
_start:
    # compute the ppn of bootpagetable
    lui t0, %hi(bootpagetable)
    li t1, 0xffffffff00000000
    sub t0, t0, t1
    srli t0, t0, 12
    # set to SV39
    # TODO: SV48 version
    li t1, (8 << 60)
    or t0, t0, t1
    # write to satp and flush TLB
    csrw satp, t0
    sfence.vma

    # load stack address
    lui sp, %hi(bootstacktop)
    addi sp, sp, %lo(bootstacktop)

    # jump to main
    lui t0, %hi(main)
    addi t0, t0, %lo(main)
    jr t0

    .section .bss.stack
    .align 12
    .global bootstack
bootstack:
    # 64KB boot stack
    .space 1024 * 64
    .global bootstacktop
bootstacktop:

    # init bootpagetable using giga page
    .section .data
    .align 12
bootpagetable:
    .quad 0
    .quad 0
    # NO.2  ：0x80000000 -> 0x80000000
    # 0xcf means DA__XWRV are all set to 1
    .quad (0x80000 << 10) | 0xcf
    .zero 507 * 8
    # NO.510：0xffffffff80000000 -> 0x80000000
    .quad (0x80000 << 10) | 0xcf
    .quad 0