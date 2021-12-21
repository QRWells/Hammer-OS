; Copyright (c) QRWells. All rights reserved.
; Licensed under the MIT license. See LICENSE file in the project root for full license information.

    org 7c00h

stack   equ 7c00h

START:
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, stack
;   clear screen
    mov ax, 0600h
    mov bx, 0700h
    mov cx, 0
    mov dx, 184fh
    int 0x10
;   set focus
    mov ax, 0200h
    mov bx, 0
    mov dx, 0
    int 0x10
;   display message
    mov bx, 0009h
    mov cx, 10
    mov ax, ds
    mov es, ax
    mov ax, 1301h
    mov bp, start_message
    int 0x10
;   reset floppy
    xor ah, ah
    xor dl, dl
    int 0x13

    jmp $

start_message   db  "Booting..."

;   fill zero
    times   510 - ($ - $$)  db  0
    dw      0xaa55