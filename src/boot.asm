; Copyright (c) QRWells. All rights reserved.
; Licensed under the MIT license. See LICENSE file in the project root for full license information.

    org 0x7c00

StartOfStack            equ     0x7c00
StartOfLoader           equ     0x1000
OffsetOfLoader          equ     0x00

RootDirSectors          equ     0xe
SectorNumOfRootDirStart equ     0x13
SectorNumOfFAT1Start    equ     0x1
SectorBalance           equ     0x11

    jmp Start
    nop

    OEMName             db      "HMOSBOOT"
    BytesPerSector      dw      0x200
    SectorsPerCluster   db      0x1
    ReservedSectors     dw      0x1
    FATs                db      0x2
    RootDirEntries      dw      0xe0
    Sectors             dw      0xb40
    MediaType           db      0xf0
    FATSectors          dw      0x9
    SectorsPerTrack     dw      0x12
    Heads               dw      0x2
    HiddenSectors       dd      0
    TolSec32            dd      0
    DriveNumber         db      0
    Reserved            db      0
    BootSignature       db      0x29
    VolumeId            dd      0
    VolumeLabel         db      "BOOT LOADER"
    FATTypeLabel        db      "FAT12   "

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

;   int 13 (CHS format)
;   al: sectors to read
;   ch: 8-LSB of track number
;   cl：sector number(0-5 bit), 2-MSB of track number (6-7 bit, only for HDD)
;   dh: head number
;   dl: drive number
;   es:bx: data buffer

;   Function: read one sector from floppy
;   ax: base sector number to read (in LBA)
;   cl: sectors to read
;   es:bx: data buffer
;   ax / SectorsPerTrack = [track:head] -> al
;   ax % SectorsPerTrack = start sector number -> ah

ReadOneSector:

    push    bp                              ; save bp & sp
    mov     bp,     sp                      ; 
    sub     esp,    2                       ; allocate 2 bytes
    mov     byte    [bp - 2],    cl         ; store cl into stack
    push    bx                              ; save bx
    mov     bl,     [SectorsPerTrack]       ; 
    div     bl
    inc     ah
    mov     cl,     ah
    mov     dh,     al
    shr     al,     1
    mov     ch,     al
    and     dh,     1
    pop     bx                              ; restore bx
    mov     dl,     [DriveNumber]
.GoOnReading:

    mov     ah,     2
    mov     al,     byte    [bp - 2]
    int     0x13
    jc      .GoOnReading                    ; if failed to read
    add     esp,    2
    pop     bp                              ; restore bp & sp
    ret

;   Function: get FAT Entry

GetFATEntry:

    push    es
    push    bx
    push    ax
    mov     ax,     00
    mov     es,     ax
    pop     ax
    mov     byte    [Odd],    0
    mov     bx,     3
    mul     bx
    mov     bx,     2
    div     bx
    cmp     dx,     0
    jz      .Even
    mov     byte    [Odd],    1

.Even:

    xor     dx,     dx
    mov     bx,     [BytesPerSector]
    div     bx
    push    dx
    mov     bx,     0x8000
    add     ax,     SectorNumOfFAT1Start
    mov     cl,     2
    call    ReadOneSector

    pop     dx
    add     bx,     dx
    mov     ax,     [es:bx]
    cmp     byte    [Odd],    1
    jnz     .Even2
    shr     ax,     4

.Even2:
    and     ax,     0x0fff
    pop     bx
    pop     es
    ret

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