; Copyright (c) QRWells. All rights reserved.
; Licensed under the MIT license. See LICENSE file in the project root for full license information.

    org 0x10000

KernelBase              equ     0x00
KernelOffset            equ     0x100000

MemoryStructBufferAddr  equ     0x7e00

    jmp Init

%include    "fat12.asm"

[SECTION    gdt]

LABELGDT:           dd      0,0
LABELDESCCODE32:    dd      0x0000ffff,0x00cf9a00
LABELDESCDATA32:    dd      0x0000ffff,0x00cf9200

GdtLen              equ     $ - LABELGDT
GdtPtr              dw      GdtLen - 1
                    dd      LABELGDT

SelectorCode32      equ     LABELDESCCODE32 - LABELGDT
SelectorData32      equ     LABELDESCDATA32 - LABELGDT

[SECTION    gdt64]

LABELGDT64:         dq      0x0000000000000000
LABELDESCCODE64:    dq      0x0020980000000000
LABELDESCDATA64:    dq      0x0000920000000000

GdtLen64            equ     $ - LABELGDT64
GdtPtr64            dw      GdtLen64 - 1
                    dd      LABELGDT64

SelectorCode64      equ     LABELDESCCODE64 - LABELGDT64
SelectorData64      equ     LABELDESCDATA64 - LABELGDT64

[SECTION    16b]
[BITS       16]

Init:

    mov     ax,     cs
    mov     ds,     ax
    mov     es,     ax
    mov     ax,     0x00
    mov     ss,     ax
    mov     sp,     0x7c00      ; set stack pointer

    ; display message

    mov     ax,     0x1301
    mov     bx,     0x000f
    mov     dx,     0x0210      ;row 2, col 16
    mov     cx,     0x11
    push    ax  
    mov     ax,     ds
    mov     es,     ax
    pop     ax  
    mov     bp,     StartLoaderMessage
    int     10h

    ; enable addressing over 1MB by int 15h
    push    ax
    mov     ax,     0x2401
    int     15h
    pop     ax

    cli                 ; disable int

    db      0x66
    lgdt    [GdtPtr]
    mov     eax,    cr0         ; set cr0 1-LSB to enter pmode
    or      eax,    1           ; |
    mov     cr0,    eax         ; v

    mov     ax,     SelectorData32
    mov     fs,     ax
    mov     eax,    cr0         ; exit pmode
    and     al,     0xfe        ; |
    mov     cr0,    eax         ; v

    sti                 ; enable int

    ; reset floppy

    push    ax
    pop     dx
    xor     ah,     ah
    xor     dl,     dl
    int     13h
    pop     dx
    pop     ax

    ; search for kernel.bin

    mov     word    [SectorNo],    SectorNumOfRootDirStart
.SearchInRootDirBegin:

    cmp     word    [RootDirSizeForLoop],   0
    jz      .NoLoaderBin                ; There is no loader.bin in all directories
    dec     word    [RootDirSizeForLoop]
    mov     ax,     0x00
    mov     es,     ax
    mov     bx,     0x8000
    mov     ax,     [SectorNo]
    mov     cl,     1
    call    ReadOneSector
    mov     si,     KernelFileName
    mov     di,     0x8000
    cld                         ; clear df because of lodsb
    mov     dx,     0x10

.SearchForLoaderBin:

    cmp     dx,     0
    jz      .GotoNextSectorInRootDir
    dec     dx
    mov     cx,     11

.CmpFileName:

    cmp     cx,     0
    jz      .FileNameFound
    dec     cx
    lodsb
    cmp     al,     byte    [es:di]
    jz      .GoOn
    jmp     .Different

.GoOn:

    inc     di
    jmp     .CmpFileName

.Different:

    and     di,     0x0ffe0
    add     di,     0x20
    mov     si,     KernelFileName
    jmp     .SearchForLoaderBin

.GotoNextSectorInRootDir:

    add     word    [SectorNo],     1
    jmp     .SearchInRootDirBegin

;   display on screen : ERROR:No LOADER Found

.NoLoaderBin:

    mov     ax,     0x1301
    mov     bx,     0x008c
    mov     dx,     0x0100
    mov     cx,     23
    push    ax
    mov     ax,     ds
    mov     es,     ax
    pop     ax
    mov     bp,     NoLoaderMessage
    int     0x10
    jmp     $

;   found loader.bin name in root director struct

.FileNameFound:

    mov     ax,     RootDirSectors
    and     di,     0x0ffe0
    add     di,     0x01a
    mov     cx,     word    [es:di]
    push    cx  
    add     cx,     ax
    add     cx,     SectorBalance
    mov     ax,     0
    mov     es,     ax
    mov     bx,     MemoryStructBufferAddr
    mov     ax,     cx

.GoOnLoadingFile:
    push    ax
    push    bx
    mov     ah,     0x0e
    mov     al,     '.'
    mov     bl,     0x0f
    int     10h  
    pop     bx   
    pop     ax   

    mov     cl,     1
    call    ReadOneSector
    pop     ax

    push    cx
    push    eax
    push    fs
    push    edi
    push    ds
    push    esi

    mov     cx,     0x200                               ; sector size used in loop
    mov     ax,     KernelBase                          ; copy for ds:esi to fs:edi byte by byte
    mov     fs,     ax                                  ; and sector by sector
    mov     edi,    dword   [OffsetOfKernelFileCount]   
    xor     ax,     ax
    mov     ds,     ax
    mov     esi,    MemoryStructBufferAddr

.CopyKernel:
    
    mov     al,     byte    [ds:esi]
    mov     byte    [fs:edi],    al

    inc     esi
    inc     edi

    loop    .CopyKernel

    mov     eax,    0x1000
    mov     ds,     eax

    mov     dword   [OffsetOfKernelFileCount],    edi

    pop     esi
    pop     ds
    pop     edi
    pop     fs
    pop     eax
    pop     cx

    call    GetFATEntry
    cmp     ax,     0x0fff
    jz      .FileLoaded
    push    ax
    mov     dx,     RootDirSectors
    add     ax,     dx
    add     ax,     SectorBalance

    jmp     .GoOnLoadingFile

.FileLoaded:
    
    mov     ax,     0x0b800
    mov     gs,     ax                      ; set "video memory" address
    mov     ah,     0x0f                    ; low 4 bit fg color, high 4 bit bg color
    mov     al,     'O'
    mov     [gs:((80 * 3 + 0) * 2)], ax    ; 3 row 1 col
    mov     al,     'K'
    mov     [gs:((80 * 3 + 1) * 2)], ax    ; 3 row 2 col

CloseFloppy:
    
    push    dx
    mov     dx,     0x03f2
    mov     al,     0    
    out     dx,     al
    pop     dx

;   get memory address size type

    mov     ax,     0x1301
    mov     bx,     0x000f
    mov     dx,     0x0400  ;row 4
    mov     cx,     24
    push    ax  
    mov     ax,     ds
    mov     es,     ax
    pop     ax  
    mov     bp,     StartGetMemStructMessage
    int     10h 

    mov     ebx,    0
    mov     ax,     0x00
    mov     es,     ax
    mov     di,     MemoryStructBufferAddr 

.GetMemStruct:

    mov     eax,    0x0e820
    mov     ecx,    20
    mov     edx,    0x534d4150
    int     15h
    jc      .GetMemFail
    add     di,     20

    cmp     ebx,    0
    jne     .GetMemStruct
    jmp     .GetMemOK

.GetMemFail:

    mov     ax,     0x1301
    mov     bx,     0x008c
    mov     dx,     0x0500  ;row 5
    mov     cx,     23
    push    ax  
    mov     ax,     ds
    mov     es,     ax
    pop     ax  
    mov     bp,     GetMemStructErrMessage
    int     10h
    jmp     $

.GetMemOK:
 
    mov     ax,     0x1301
    mov     bx,     0x000f
    mov     dx,     0x0600  ;row 6
    mov     cx,     29
    push    ax
    mov     ax,     ds
    mov     es,     ax
    pop     ax
    mov     bp,     GetMemStructOKMessage
    int     10h 

;   get SVGA information

    mov     ax,     0x1301
    mov     bx,     0x000f
    mov     dx,     0x0800  ;row 8
    mov     cx,     23
    push    ax  
    mov     ax,     ds
    mov     es,     ax
    pop     ax  
    mov     bp,     StartGetSVGAVBEInfoMessage
    int     10h

    mov     ax,     0x00
    mov     es,     ax
    mov     di,     0x8000
    mov     ax,     0x4F00

    int     10h 

    cmp     ax,     0x004F

    jz      .KO
 
;   Fail

    mov     ax,     0x1301
    mov     bx,     0x008C
    mov     dx,     0x0900  ;row 9
    mov     cx,     23
    push    ax  
    mov     ax,     ds
    mov     es,     ax
    pop     ax  
    mov     bp,     GetSVGAVBEInfoErrMessage
    int     10h

    jmp     $

.KO:

    mov     ax,     0x1301
    mov     bx,     0x000F
    mov     dx,     0x0A00  ;row 10
    mov     cx,     29
    push    ax  
    mov     ax,     ds
    mov     es,     ax
    pop     ax
    mov     bp,     GetSVGAVBEInfoOKMessage
    int     10h

;    Get SVGA Mode Info

    mov     ax,     0x1301
    mov     bx,     0x000F
    mov     dx,     0x0C00  ;row 12
    mov     cx,     24
    push    ax
    mov     ax,     ds
    mov     es,     ax
    pop     ax
    mov     bp,     StartGetSVGAModeInfoMessage
    int     10h


    mov     ax,     0x00
    mov     es,     ax
    mov     si,     0x800e

    mov     esi,    dword [es:si]
    mov     edi,    0x8200

.SVGAModeInfoGet:

    mov     cx,     word [es:esi]  

;    display SVGA mode information

    push    ax
    
    mov     ax,     00h
    mov     al,     ch
    call    DispAL

    mov     ax,     00h
    mov     al,     cl 
    call    DispAL
    
    pop     ax

;   
 
    cmp     cx,     0x0FFFF
    jz      .SVGAModeInfoFinish

    mov     ax,     0x4F01
    int     10h

    cmp     ax,     0x004F

    jnz     .SVGAModeInfoFAIL 

    add     esi,    2
    add     edi,    0x100

    jmp     .SVGAModeInfoGet
  
.SVGAModeInfoFAIL:

    mov     ax,     0x1301
    mov     bx,     0x008C
    mov     dx,     0x0D00  ;row 13
    mov     cx,     24
    push    ax
    mov     ax,     ds
    mov     es,     ax
    pop     ax
    mov     bp,     GetSVGAModeInfoErrMessage
    int     10h

.SETSVGAModeVESAVBEFAIL:

    jmp     $

.SVGAModeInfoFinish:

    mov     ax,     0x1301
    mov     bx,     0x000F
    mov     dx,     0x0E00  ;row 14
    mov     cx,     30
    push    ax
    mov     ax,     ds
    mov     es,     ax
    pop     ax
    mov     bp, GetSVGAModeInfoOKMessage
    int     10h

;   set the SVGA mode(VESA VBE)

    mov     ax,     0x4F02
    mov     bx,     0x4180 ;========================mode : 0x180 or 0x143
    int     10h

    cmp     ax,     0x004F
    jnz     .SETSVGAModeVESAVBEFAIL

;   init IDT GDT goto protect mode 

    cli

            db      0x66
    lgdt    [GdtPtr]

;           db      0x66
;   lidt    [IDTPOINTER]

    mov     eax,    cr0
    or      eax,    1
    mov     cr0,    eax

    jmp     dword   SelectorCode32:GoToTmpProtect

[SECTION    32b]
[BITS       32]

GoToTmpProtect:

;   go to tmp long mode

    mov     ax,     0x10
    mov     ds,     ax
    mov     es,     ax
    mov     fs,     ax
    mov     ss,     ax
    mov     esp,    0x7e00

    call    supportlongmode
    test    eax,    eax

    jz      nosupport

;   init temporary page table 0x90000

    mov     dword [0x90000],    0x91007
    mov     dword [0x90004],    0x00000
    mov     dword [0x90800],    0x91007
    mov     dword [0x90804],    0x00000

    mov     dword [0x91000],    0x92007
    mov     dword [0x91004],    0x00000

    mov     dword [0x92000],    0x000083
    mov     dword [0x92004],    0x000000

    mov     dword [0x92008],    0x200083
    mov     dword [0x9200c],    0x000000

    mov     dword [0x92010],    0x400083
    mov     dword [0x92014],    0x000000

    mov     dword [0x92018],    0x600083
    mov     dword [0x9201c],    0x000000

    mov     dword [0x92020],    0x800083
    mov     dword [0x92024],    0x000000

    mov     dword [0x92028],    0xa00083
    mov     dword [0x9202c],    0x000000

;   load GDTR

            db      0x66
    lgdt    [GdtPtr64]
    mov     ax,     0x10
    mov     ds,     ax
    mov     es,     ax
    mov     fs,     ax
    mov     gs,     ax
    mov     ss,     ax

    mov     esp,    0x7E00

;   open PAE

    mov     eax,    cr4
    bts     eax,    5
    mov     cr4,    eax

;   load cr3

    mov     eax,    0x90000
    mov     cr3,    eax

;    enable long-mode

    mov     ecx,    0x0C0000080  ;IA32EFER
    rdmsr

    bts     eax,    8
    wrmsr

;   open PE and paging

    mov     eax,    cr0
    bts     eax,    0
    bts     eax,    31
    mov     cr0,    eax

    jmp     SelectorCode64:KernelOffset

;   test support long mode or not

supportlongmode:

    mov     eax,    0x80000000
    cpuid
    cmp     eax,    0x80000001
    setnb   al 
    jb      supportlongmodedone
    mov     eax,    0x80000001
    cpuid
    bt      edx,    29
    setc    al
supportlongmodedone:
 
    movzx   eax,    al
    ret

;   no support

nosupport:
    jmp     $

[SECTION .s16lib]
[BITS 16]

%include "lib.asm"

;   tmp IDT

IDT:
    times   0x50    dq  0
IDT_END:

IDT_POINTER:
    dw      IDT_END - IDT - 1
    dd      IDT

RootDirSizeForLoop          dw  RootDirSectors
SectorNo                    dw  0
Odd                         db  0
OffsetOfKernelFileCount     dd  KernelOffset

DisplayPosition             dd  0

StartLoaderMessage:         db  "Loading kernel..."
NoLoaderMessage:            db  "ERROR:No KERNEL Found"
KernelFileName:             db  "KERNEL  BIN",  0

StartGetMemStructMessage:   db  "Start Get Memory Struct."
GetMemStructErrMessage:     db  "Get Memory Struct ERROR"
GetMemStructOKMessage:      db  "Get Memory Struct SUCCESSFUL!"

StartGetSVGAVBEInfoMessage: db  "Start Get SVGA VBE Info"
GetSVGAVBEInfoErrMessage:   db  "Get SVGA VBE Info ERROR"
GetSVGAVBEInfoOKMessage:    db  "Get SVGA VBE Info SUCCESSFUL!"

StartGetSVGAModeInfoMessage:db  "Start Get SVGA Mode Info"
GetSVGAModeInfoErrMessage:  db  "Get SVGA Mode Info ERROR"
GetSVGAModeInfoOKMessage:   db  "Get SVGA Mode Info SUCCESSFUL!"