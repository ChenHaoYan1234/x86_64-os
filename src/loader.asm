    org 10000h

    jmp Label_Start

%include "fat12.inc"

Label_Start:
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ax, 0x00
    mov ss, ax
    mov sp, 0x7c00

; display on screen: Start Loader
    mov ax, 1301h
    mov bx, 000fh
    mov cx, 12
    mov dx, 0200h
    push ax
    mov ax, ds
    mov es, ax
    pop ax
    mov bp, StartLoaderMessage
    int 10h

    jmp $

; messages
StartLoaderMessage: db "Start Loader"
