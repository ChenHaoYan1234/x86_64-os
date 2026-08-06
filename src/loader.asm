    org 10000h

    jmp Label_Start

%include "fat12.inc"

BaseOfKernelFile equ 0x00
OffsetOfKernelFile equ 0x100000

BaseTmpOfKernelAddr equ 0x00
OffsetTmpOfKernelAddr equ 0x7e00

MemoryStructBufferAddr equ 0x7e00

[SECTION gdt]
LABEL_GDT dd 0, 0
LABEL_DESC_CODE32 dd 0x0000ffff, 0x00cf9a00
LABEL_DESC_DATA32 dd 0x0000ffff, 0x00cf9200

GdtLen equ $ - LABEL_GDT
GdtPtr dw GdtLen - 1
    dd LABEL_GDT

SelectorCode32 equ LABEL_DESC_CODE32 - LABEL_GDT
SelectorData32 equ LABEL_DESC_DATA32 - LABEL_GDT

; tmp IDT
IDT:
    times 0x50 dq 0
IDT_END:

IDT_POINTER:
    dw IDT_END - IDT - 1
    dd IDT


[SECTION .s16]
[BITS 16]

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

; open address A20
    push ax
    in al, 92h
    or al, 00000010b
    out 92h, al
    pop ax
    cli

    db 0x66
    lgdt [GdtPtr]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    mov ax, SelectorData32
    mov fs, ax
    mov eax, cr0
    and al, 11111110b
    mov cr0, eax

    sti

; search loader.bin
    mov word [SectorNo], SectorNumOfRootDirStart

Label_Search_In_Root_Dir_Begin:
    cmp word [RootDirSizeForLoop], 0
    jz Label_No_KernelBin
    dec word [RootDirSizeForLoop]
    mov ax, 00h
    mov es, ax
    mov bx, 8000h
    mov ax, [SectorNo]
    mov cl, 1
    call Func_ReadOneSector
    mov si, KernelFileName
    mov di, 8000h
    cld
    mov dx, 10h

Label_Search_For_KernelBin:
    cmp dx, 0
    jz Label_Goto_Next_Sector_In_Root_Dir
    dec dx
    mov cx, 11

Label_Cmp_FileName:
    cmp cx, 0
    jz Label_FileName_Found
    dec cx
    lodsb
    cmp al, byte [es:di]
    jz Label_Go_On
    jmp Label_Different

Label_Go_On:
    inc di
    jmp Label_Cmp_FileName

Label_Different:
    and di, 0ffe0h
    add di, 20h
    mov si, KernelFileName
    jmp Label_Search_For_KernelBin

Label_Goto_Next_Sector_In_Root_Dir:
    add word [SectorNo], 1
    jmp Label_Search_In_Root_Dir_Begin

; display on screen: ERROR:No KERNEL Found
Label_No_KernelBin:
    mov ax, 1301h
    mov bx, 008ch
    mov cx, 21
    mov dx, 0300h
    push ax
    mov ax, ds
    mov es, ax
    pop ax
    mov bp, NoLoaderMessage
    int 10h
    jmp $

Label_FileName_Found:
    mov ax, RootDirSectors
    and di, 0ffe0h
    add di, 01ah
    mov cx, word [es:di]
    push cx
    add cx, ax
    add cx, SectorBalance
    mov eax, BaseTmpOfKernelAddr
    mov es, eax
    mov bx, OffsetTmpOfKernelAddr
    mov ax, cx

Label_Go_On_Loading_File:
    push ax
    push bx
    mov ah, 0Eh
    mov al, '.'
    mov bl, 0Fh
    int 10h
    pop bx
    pop ax

    mov cl, 1
    call Func_ReadOneSector
    pop ax

    push cx
    push eax
    push fs
    push edi
    push ds
    push esi

    mov cx, 200h
    mov ax, BaseOfKernelFile
    mov fs, ax
    mov edi, dword [OffsetOfKernelFileCount]

    mov ax, BaseTmpOfKernelAddr
    mov ds, ax
    mov esi, OffsetTmpOfKernelAddr

Label_Mov_Kernel:
    mov al, byte [ds:esi]
    mov byte [fs:edi], al

    inc esi
    inc edi

    loop Label_Mov_Kernel

    mov eax, 0x1000
    mov ds, eax

    mov dword [OffsetOfKernelFileCount], edi
    pop esi
    pop ds
    pop edi
    pop fs
    pop eax
    pop cx

    call Func_GetFATEntry
    cmp ax, 0FFFh
    jz Label_File_Loaded
    push ax
    mov dx, RootDirSectors
    add ax, dx
    add ax, SectorBalance

    jmp Label_Go_On_Loading_File

Label_File_Loaded:
    mov ax, 0b800h
    mov gs, ax
    mov ah, 0fh
    mov al, 'G'
    mov [gs:((80*0 + 39) * 2)], ax

KillMotor:
    push dx
    mov dx, 03f2h
    mov al, 0
    out dx, al
    pop dx

; get memory address size type
    mov ax, 1301h
    mov bx, 000fh
    mov cx, 24
    mov dx, 0400h
    push ax
    mov ax, ds
    mov es, ax
    pop ax
    mov bp, StartGetMemStructMessage
    int 10h

    mov ebx, 0
    mov ax, 0x00
    mov es, ax
    mov di, MemoryStructBufferAddr

Label_Get_Mem_Struct:
    mov eax, 0x0e820
    mov ecx, 20
    mov edx, 0x534d4150
    int 15h
    jc Label_Get_Mem_Failed
    add di, 20

    cmp ebx, 0
    jne Label_Get_Mem_Struct
    jmp Label_Get_Mem_OK

Label_Get_Mem_Failed:
    mov ax, 1301h
    mov bx, 008ch
    mov cx, 23
    mov dx, 0500h
    push ax
    mov ax, ds
    mov es, ax
    pop ax
    mov bp, GetMemStructErrMessage
    int 10h
    jmp $

Label_Get_Mem_OK:
    mov ax, 1301h
    mov bx, 000fh
    mov cx, 29
    mov dx, 0600h
    push ax
    mov ax, ds
    mov es, ax
    pop ax
    mov bp, GetMemStructOKMessage
    int 10h

; get svga information
    mov ax, 1301h
    mov bx, 000fh
    mov cx, 23
    mov dx, 0800h
    push ax
    mov ax, ds
    mov es, ax
    pop ax
    mov bp, StartGetSVGAVBEInfoMessage
    int 10h

    mov ax, 0x00
    mov es, ax
    mov di, 0x8000
    mov ax, 4f00h

    int 10h

    cmp ax, 004fh

    jz .KO

    mov ax, 1301h
    mov bx, 008ch
    mov cx, 23
    mov dx, 0900h
    push ax
    mov ax, ds
    mov es, ax
    pop ax
    mov bp, GetSVGAVBEInfoErrMessage
    int 10h

    jmp $

.KO:
    mov ax, 1301h
    mov bx, 000fh
    mov cx, 29
    mov dx, 0a00h
    push ax
    mov ax, ds
    mov es, ax
    pop ax
    mov bp, GetSVGAVBEInfoOKMessage
    int 10h

; get svga mode information
    mov ax, 1301h
    mov bx, 000fh
    mov cx, 24
    mov dx, 0c00h
    push ax
    mov ax,ds
    mov es, ax
    pop ax
    mov bp, StartGetSVGAModeInfoMessage
    int 10h

    mov ax, 0x00
    mov es, ax
    mov si, 0x800e

    mov esi, dword [es:si]
    mov edi, 0x8200

Label_SVGA_Mode_Info_Get:
    mov cx, word [es:esi]

    push ax

    mov ax, 00h
    mov al, ch
    call Func_DispAL

    mov ax, 00h
    mov al, cl
    call Func_DispAL

    pop ax

    cmp cx, 0ffffh
    jz Label_SVGA_Mode_Info_Finish

    mov ax, 4f01h
    int 10h

    cmp ax, 004fh

    jnz Label_SVGA_Mode_Info_Failed

    add esi, 2
    add edi, 0x100

    jmp Label_SVGA_Mode_Info_Get

Label_SVGA_Mode_Info_Failed:
    mov ax, 1301h
    mov bx, 008ch
    mov cx, 24
    mov dx, 0d00h
    push ax
    mov ax, ds
    mov es, ax
    pop ax
    mov bp, GetSVGAModeInfoErrMessage
    int 10h

Label_SET_SVGA_Mode_VESA_VBE_Failed:
    jmp $

Label_SVGA_Mode_Info_Finish:
    mov ax, 1301h
    mov bx, 000fh
    mov cx, 30
    mov dx, 0e00h
    push ax
    mov ax, ds
    mov es, ax
    pop ax
    mov bp, GetSVGAModeInfoOKMessage
    int 10h

; set the SVGA mode (VESA VBE)
    mov ax, 4f02h
    mov bx, 4143h
    int 10h

    cmp ax, 004fh
    jnz Label_SET_SVGA_Mode_VESA_VBE_Failed

; init IDT GDT goto protect mode
    cli

    db 0x66
    lgdt [GdtPtr]

;    db 0x66
;    lidt [IDT_POINTER]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp dword SelectorCode32:GO_TO_TMP_Protect

GO_TO_TMP_Protect:
    jmp $

[SECTION .s16lib]
[BITS 16]
; read on sector from floppy
Func_ReadOneSector:
    push bp
    mov bp, sp
    sub esp, 2
    mov byte [bp - 2], cl
    push bx
    mov bl, [BPB_SecPerTrk]
    div bl
    inc ah
    mov cl, ah
    mov dh, al
    shr al, 1
    mov ch, al
    and dh, 1
    pop bx
    mov dl, [BS_DrvNum]
Label_Go_On_Reading:
    mov ah, 2
    mov al, byte [bp - 2]
    int 13h
    jc Label_Go_On_Reading
    add esp, 2
    pop bp
    ret

; get FAT entry
Func_GetFATEntry:
    push es
    push bx
    push ax
    mov ax, 0
    mov es, ax
    pop ax
    mov byte [Odd], 0
    mov bx, 3
    mul bx
    mov bx, 2
    div bx
    cmp dx, 0
    jz Label_Even
    mov byte [Odd], 1

Label_Even:
    xor dx, dx
    mov bx, [BPB_BytesPerSec]
    div bx
    push dx
    mov bx, 8000h
    add ax, SectorNumOfFAT1Start
    mov cl, 2
    call Func_ReadOneSector

    pop dx
    add bx, dx
    mov ax, [es:bx]
    cmp byte [Odd], 1
    jnz Label_Even_2
    shr ax, 4

Label_Even_2:
    and ax, 0fffh
    pop bx
    pop es
    ret

Func_DispAL:
    push ecx
    push edx
    push edi

    mov edi, [DisplayPosition]
    mov ah, 0fh
    mov dl, al
    mov al, 4
    mov ecx, 2
.begin:
    and al, 0fh
    cmp al, 9
    ja .1
    add al, '0'
    jmp .2
.1:
    sub al, 0ah
    add al, 'A'
.2:
    mov [gs:edi], ax
    add edi, 2
    mov al, dl
    loop .begin

    mov [DisplayPosition], edi

    pop edi
    pop edx
    pop ecx

    ret

; tmp variables
RootDirSizeForLoop dw RootDirSectors
SectorNo dw 0
Odd db 0
OffsetOfKernelFileCount dd OffsetOfKernelFile
DisplayPosition dd 0

; messages
StartLoaderMessage: db "Start Loader"
NoLoaderMessage: db "ERROR:No KERNEL Found"
KernelFileName: db "KERNEL  BIN", 0

StartGetMemStructMessage: db "Start Get Memory Struct."
GetMemStructErrMessage: db "Get Memory Struct ERROR"
GetMemStructOKMessage: db "Get Memory Struct SUCCESSFUL!"

StartGetSVGAVBEInfoMessage: db "Start Get SVGA VBE Info"
GetSVGAVBEInfoErrMessage: db "Get SVGA VBE Info ERROR"
GetSVGAVBEInfoOKMessage: db "Get SVGA VBE Info SUCCESSFUL!"

StartGetSVGAModeInfoMessage: db "Start Get SVGA Mode Info"
GetSVGAModeInfoErrMessage: db "Get SVGA Mode Info ERROR"
GetSVGAModeInfoOKMessage: db "Get SVGA Mode Info SUCCESSFUL!"
