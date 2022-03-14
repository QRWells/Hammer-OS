; Copyright (c) QRWells. All rights reserved.
; Licensed under the MIT license. See LICENSE file in the project root for full license information.

    org 10000h

    mov     ax, cs
    mov     ds, ax
    mov     es, ax
    mov     ax, 0x00
    mov     ss, ax
    mov     sp, 0x7c00                  ; set stack pointer

    ; display message

    mov     ax, 1301h
    mov     bx, 000fh
    mov     dx, 0200h  ;row 2
    mov     cx, 12
    push    ax
    mov     ax, ds
    mov     es, ax
    pop     ax
    mov     bp, StartLoaderMessage
    int     10h

    jmp $

    ; display messages

StartLoaderMessage: db "Loading..."