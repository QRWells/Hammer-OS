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

; display num in al

DispAL:

    push ecx
    push edx
    push edi
    
    mov     edi, [DisplayPosition]
    mov     ah,     0Fh
    mov     dl,     al
    shr     al,     4
    mov     ecx,    2
.begin: 

    and     al,     0Fh
    cmp     al,     9
    ja      .1
    add     al,     '0'
    jmp     .2  
.1: 

    sub     al,     0Ah
    add     al,     'A'
.2:

    mov     [gs:edi],   ax
    add     edi, 2
    
    mov     al,     dl
    loop    .begin

    mov     [DisplayPosition], edi

    pop     edi
    pop     edx
    pop     ecx
    
    ret