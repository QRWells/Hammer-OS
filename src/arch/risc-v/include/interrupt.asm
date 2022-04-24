.altmacro
# register has 8 bytes length
.set    REG_SIZE, 8
# context size is 34 * 8 bytes
.set    CONTEXT_SIZE, 34

# macros for saving registers
.macro SAVE reg, offset
    sd  \reg, \offset*8(sp)
.endm

.macro SAVE_N n
    SAVE  x\n, \n
.endm

# macros for restoring registers
.macro LOAD reg, offset
    ld  \reg, \offset*8(sp)
.endm

.macro LOAD_N n
    LOAD  x\n, \n
.endm


    .section .text
    .globl __interrupt
    # interrupt handler needs 4 bytes align
    .balign 4
# global interrupt handler, save context and jump to handle_interrupt()
__interrupt:
    # allocate space for context
    addi    sp, sp, -34*REG_SIZE
    
    SAVE    x1, 1
    addi    x1, sp, 34*REG_SIZE
    SAVE    x1, 2
    .set    n, 3
    .rept   29
        SAVE_N  %n
        .set    n, n + 1
    .endr

    # save CSRs
    csrr    s1, sstatus
    csrr    s2, sepc
    SAVE    s1, 32
    SAVE    s2, 33

    # call handle_interrupt()
    # pass context address(stack top), scause and stval
    mv      a0, sp
    csrr    a1, scause
    csrr    a2, stval
    jal     handle_interrupt


# Return from handle_interrupt()
# Restore all registers from Context, and jump to sepc in context
    .globl __restore
__restore:
    # restore CSR
    LOAD    s1, 32
    LOAD    s2, 33
    csrw    sstatus, s1
    csrw    sepc, s2

    LOAD    x1, 1
    .set    n, 3
    .rept   29
        LOAD_N  %n
        .set    n, n + 1
    .endr

    # restore sp
    LOAD    x2, 2
    sret
