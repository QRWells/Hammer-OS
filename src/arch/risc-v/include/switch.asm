.equ    REG_SIZE, 8
.altmacro
.macro SAVE reg, offset
  sd  \reg, \offset*REG_SIZE(sp)
.endm

.macro SAVE_N n
  SAVE  s\n, (\n+2)
.endm

.macro LOAD reg, offset
    ld  \reg, \offset*REG_SIZE(sp)
.endm

.macro LOAD_N n
    LOAD  s\n, (\n+2)
.endm

    addi    sp, sp, (-REG_SIZE*14)
    sd      sp, 0(a0)
    SAVE    ra, 0
    .set    n, 0
    .rept   12
        SAVE_N  %n
        .set    n, n + 1
    .endr
    csrr    s11, satp
    SAVE    s11, 1

    ld      sp, 0(a1)
    LOAD    s11, 1
    csrw    satp, s11
    sfence.vma
    LOAD    ra, 0
    .set    n, 0
    .rept   12
        LOAD_N  %n
        .set    n, n + 1
    .endr
    addi    sp, sp, (REG_SIZE*14)
    
    sd      zero, 0(a1)
    ret
