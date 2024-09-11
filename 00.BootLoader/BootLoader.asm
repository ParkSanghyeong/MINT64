    [ORG 0x00]
    [BITS 16]

    SECTION .text

    jmp 0x07c0:START

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Environment
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
TOTALSECTORCOUNT:   dw 0x02

KERNEL32SECTORCOUNT: dw 0x02

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Code
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
START:
    mov ax, 0x07c0  ; Boot Loader(0x7c00)
    mov ds, ax
    mov ax, 0xb800  ; Video Memory(0xb8000)
    mov es, ax

    ; stack setting
    ; dangerous. stack overflow..
    mov ax, 0x0000
    mov ss, ax
    mov sp, 0x0000      ; when push is called, sp overflows to 0xfffe.
                        ; so the stack uses the entire memory size.
    mov bp, 0xfffe      ; memory align.. even idx read is faster than odd one.
                        ; 
                        ; https://jsandroidapp.cafe24.com/xe/qna/5442

    ; ===================================================
    ; clear screen
    ; ===================================================
    mov si, 0

.SCREENCLEARLOOP:
    mov byte [es:si], 0         ; char
    mov byte [es:si+1], 0x0f    ; attribute
    add si, 2
    cmp si, 80*25*2
    jl .SCREENCLEARLOOP

    ; ===================================================
    ; Start Message
    ; ===================================================
    push MESSAGE1
    push 0
    push 0
    call PRINTMESSAGE
    add sp, 6

    ; ===================================================
    ; Image loading Message
    ; ===================================================
    push IMAGELOADINGMESSAGE
    push 0
    push 1
    call PRINTMESSAGE
    add sp, 6

    ; ----------------------------------------------------
    ; Load OS image
    ; ----------------------------------------------------

    ; ===================================================
    ; Reset disk
    ; ===================================================
RESETDISK:
    mov ax, 0
    mov dl, 0
    int 0x13

    jc HANDLEDISKERROR

    ; ===================================================
    ; Read sector from disk
    ; ===================================================
    mov si, 0x1000
    mov es, si
    mov bx, 0x0000

    mov di, word [TOTALSECTORCOUNT]

READDATA:
    cmp di, 0
    je READEND
    sub di, 0x1

    ; ===================================================
    ; Call BIOS read function
    ; ===================================================
    mov ah, 0x02
    mov al, 0x1
    mov ch, byte [TRACKNUMBER]
    mov cl, byte [SECTORNUMBER]
    mov dh, byte [HEADNUMBER]
    mov dl, 0x00
    int 0x13
    jc HANDLEDISKERROR

    ; ===================================================
    ; Calcurate track, head, sector address
    ; ===================================================
    add si, 0x0020
    mov es, si

    mov al, byte [SECTORNUMBER]
    add al, 0x01
    mov byte[SECTORNUMBER], al
    cmp al, 37
    jl READDATA

    xor byte [HEADNUMBER], 0x01
    mov byte [SECTORNUMBER], 0x01

    cmp byte [HEADNUMBER], 0x00
    jne READDATA

    add byte [TRACKNUMBER], 0x01
    jmp READDATA

READEND:
    ; ===================================================
    ; Image loading complete Message
    ; ===================================================
    push LOADINGCOMPLETEMESSAGE
    push 20
    push 1
    call PRINTMESSAGE
    add sp, 6

    jmp 0x1000:0x0000   ; execute OS image

; ===================================================================
; functions
; ===================================================================

; Handle disk error
HANDLEDISKERROR:
    push DISKERRORMESSAGE
    push 20
    push 1
    call PRINTMESSAGE
    
    jmp $
; --------------------------------

;   Print message
;   (params: pos y, pos x, string)
PRINTMESSAGE: 
    push bp
    mov bp, sp

    push es     ; save regs for restore
    push si
    push di
    push ax
    push cx
    push dx

    mov ax, 0xb800
    mov es, ax

    ; (param): pos y
    mov ax, word [bp+4]
    mov si, 160         ; single line length(byte)
    mul si
    mov di, ax

    ; (param): pos x
    mov ax, word [bp+6]
    mov si, 2           ; single text length(byte)
    mul si
    add di, ax

    ; (param): string
    mov si, word [bp+8]

.MESSAGELOOP:
    mov cl, byte [si]
    cmp cl, 0
    je .MESSAGEEND

    mov byte[es:di], cl

    add si, 1
    add di, 2

    jmp .MESSAGELOOP

.MESSAGEEND:
    pop dx  ; restore regs
    pop cx
    pop ax
    pop di
    pop si
    pop es
    pop bp
    ret
; --------------------------------

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Data
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

MESSAGE1: db 'MINT64 OS Boot Loader Start.', 0;

DISKERRORMESSAGE:       db 'Disk Error.', 0
IMAGELOADINGMESSAGE     db 'OS Image Loading...', 0
LOADINGCOMPLETEMESSAGE  db 'Complete.', 0

SECTORNUMBER:           db 0x02
HEADNUMBER:             db 0x00
TRACKNUMBER:            db 0x00

    times 510 - ($ - $$) db 0x00

    db 0x55
    db 0xaa
