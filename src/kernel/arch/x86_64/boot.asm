; Copyright (c) QRWells. All rights reserved.
; Licensed under the MIT license. See LICENSE file in the project root for full license information.

    org 0x7c00

StartOfStack            equ     0x7c00
StartOfLoader           equ     0x1000
OffsetOfLoader          equ     0x00

    jmp Start
    nop

    %include "fat12.asm"

Start:
    mov     ax,     cs
    mov     ds,     ax
    mov     es,     ax
    mov     ss,     ax
    mov     sp,     StartOfStack

    ;   clear screen
    mov     ax,     0x0600
    mov     bx,     0x0700
    mov     cx,     0
    mov     dx,     0x184f
    int     0x10

    ;   set cursor
    mov     ax,     0x0200
    mov     bx,     0
    mov     dx,     0
    int     0x10
    
    ;   display message
    mov     bx,     0x0009
    mov     cx,     10
    mov     ax,     ds
    mov     es,     ax
    mov     ax,     0x1301
    mov     bp,     StartMessage
    int     0x10

;   reset floppy
    xor     ah,     ah
    xor     dl,     dl
    int     0x13

;   search loader.bin
    mov     word    [SectorNo], SectorNumOfRootDirStart

.SearchInRootDirBegin:

    cmp     word    [RootDirSizeForLoop],   0
    jz      .NoLoaderBin                            ; There is no loader.bin in all directories
    dec     word    [RootDirSizeForLoop]
    mov     ax,     0x00
    mov     es,     ax
    mov     bx,     0x8000
    mov     ax,     [SectorNo]
    mov     cl,     1
    call    ReadOneSector
    mov     si,     LoaderFileName
    mov     di,     0x8000
    cld                                             ; clear df because of lodsb
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
    mov     si,     LoaderFileName
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

    mov     ax,    RootDirSectors
    and     di,    0x0ffe0
    add     di,    0x01a
    mov     cx,    word    [es:di]
    push    cx
    add     cx,    ax
    add     cx,    SectorBalance
    mov     ax,    StartOfLoader
    mov     es,    ax
    mov     bx,    OffsetOfLoader
    mov     ax,    cx

.GoOnLoadingFile:

    mov     cl,    1
    call    ReadOneSector
    pop     ax
    call    GetFATEntry
    cmp     ax,    0x0fff
    jz      .FileLoaded
    push    ax
    mov     dx,    RootDirSectors
    add     ax,    dx
    add     ax,    SectorBalance
    add     bx,    [BytesPerSector]
    jmp     .GoOnLoadingFile

.FileLoaded:

    jmp     StartOfLoader:OffsetOfLoader

    %include "lib.asm"

;   tmp variable

RootDirSizeForLoop  dw    RootDirSectors
SectorNo            dw    0
Odd                 db    0

;   display messages

StartMessage:      db    "Booting..."
NoLoaderMessage:    db    "ERROR: LOADER not found"
LoaderFileName:     db    "LOADER  BIN",    0

;   fill zero until whole sector

    times   0x1fe - ($ - $$)    db      0
    dw      0xaa55