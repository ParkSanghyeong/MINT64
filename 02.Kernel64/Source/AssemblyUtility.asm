    [BITS 64]
    SECTION .text

global kInPortByte, kOutPortByte, kLoadGDTR, kLoadTR, kLoadIDTR
global kEnableInterrupt, kDisableInterrupt, kReadRFLAGS
global kReadTSC
global kSwitchContext, kHlt

kInPortByte:
    push rdx

    mov rdx, rdi
    mov rax, 0
    in al, dx

    pop rdx
    ret

kOutPortByte:
    push rdx
    push rax

    mov rdx, rdi
    mov rax, rsi
    out dx, al

    pop rax
    pop rdx
    ret

kLoadGDTR:
    lgdt [rdi]
    ret

kLoadTR:
    ltr di
    ret

kLoadIDTR:
    lidt [rdi]
    ret

kEnableInterrupt:
    sti
    ret

kDisableInterrupt:
    cli
    ret

kReadRFLAGS:
    pushfq
    pop rax
    ret

kReadTSC:
    push rdx

    rdtsc

    shl rdx, 32
    or rax, rdx

    pop rdx
    ret

%macro KSAVECONTEXT 0
    push rbp
    push rax
    push rbx
    push rcx
    push rdx
    push rdi
    push rsi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov ax, ds
    push rax
    mov ax, es
    push rax

    push fs
    push gs
%endmacro

%macro KLOADCONTEXT 0 
    pop gs
    pop fs

    pop rax
    mov es, ax
    pop rax 
    mov ds, ax

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rsi
    pop rdi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    pop rbp
%endmacro

kSwitchContext:
    push rbp
    mov rbp, rsp

    ; if current context is null
    pushfq      
    cmp rdi, 0
    je .LoadContext
    popfq               ;restore flags

    push rax
    
    mov ax, ss
    mov qword[rdi + (23*8)], rax

    mov rax, rbp
    add rax, 16
    mov qword[rdi + (22*8)], rax

    pushfq
    pop rax
    mov qword[rdi + (21*8)], rax

    mov ax, cs
    mov qword[rdi + (20*8)], rax

    mov rax, qword[rbp + 8]
    mov qword[rdi + (19*8)], rax

    ; restore registers
    pop rax
    pop rbp

    ; ======= SAVE CONTEXT =======
    ; adjust stack pointer
    add rdi, (19*8)
    mov rsp, rdi
    sub rdi, (19*8) ; restore rdi

    KSAVECONTEXT

.LoadContext:
    mov rsp, rsi

    ; load context
    KLOADCONTEXT
    iretq

kHlt:
    hlt
    hlt
    ret
    