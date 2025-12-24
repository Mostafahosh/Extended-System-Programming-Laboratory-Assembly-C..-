section .text
global _start
global system_call

_start:
    pop    dword ecx
    mov    esi,esp
    mov     eax,ecx
    shl     eax,2
    add     eax,esi
    add     eax,4
    push    dword eax
    push    dword esi
    push    dword ecx
    call    main
    mov     ebx,eax
    mov     eax,1
    int     0x80
    nop
        
system_call:
    push    ebp
    mov     ebp, esp
    sub     esp, 4
    pushad
    mov     eax, [ebp+8]
    mov     ebx, [ebp+12]
    mov     ecx, [ebp+16]
    mov     edx, [ebp+20]
    int     0x80
    mov     [ebp-4], eax
    popad
    mov     eax, [ebp-4]
    add     esp, 4
    pop     ebp
    ret

; =========================
; Task 1.A starts here
; =========================

global main
extern strlen

section .data
newline db 10

Infile  dd 0      ; stdin
Outfile dd 1      ; stdout

section .bss
buf resb 1        ; 1-byte buffer

section .text

global main

main:
    push ebp
    mov  ebp, esp

    mov  ecx, [ebp+8]      ; argc
    mov  esi, [ebp+12]     ; argv
    mov  edi, 1            ; i = 1 (skip argv[0])

.arg_loop:
    cmp  edi, ecx
    jge  .done_args

    mov  eax, [esi + edi*4]    ; eax = argv[i]

    cmp  byte [eax], '-'
    jne  .next_arg

    cmp  byte [eax+1], 'i'
    je   .handle_input

    cmp  byte [eax+1], 'o'
    je   .handle_output

    jmp  .next_arg

.handle_input:
    add  eax, 2                ; filename
    push 0                     ; mode (unused)
    push 0                     ; O_RDONLY
    push eax                   ; filename
    push 5                     ; SYS_OPEN
    call system_call
    add  esp, 16
    cmp  eax, 0
    jl   .next_arg
    mov  [Infile], eax
    jmp  .next_arg

.handle_output:
    add  eax, 2                ; filename
    push 0644                  ; mode
    push 577                   ; flags
    push eax                   ; filename
    push 5                     ; SYS_OPEN
    call system_call
    add  esp, 16
    cmp  eax, 0
    jl   .next_arg
    mov  [Outfile], eax

.next_arg:
    inc  edi
    jmp  .arg_loop

.done_args:
    call encode

    mov  eax, 0
    leave
    ret

; =========================
; Task 1.B 
; =========================
global encode

encode:
.read_loop:
    ; read 1 byte
    push 1
    push buf
    push dword [Infile]
    push 3              ; SYS_READ
    call system_call
    add  esp, 16

    cmp  eax, 0
    je   .done           ; EOF

    mov  al, [buf]

    ; if 'A' <= al <= 'Z', add 3
    cmp  al, 'A'
    jl   .write
    cmp  al, 'Z'
    jg   .write
    add  al, 3
    mov  [buf], al

.write:
    push 1
    push buf
    push dword [Outfile]
    push 4              ; SYS_WRITE
    call system_call
    add  esp, 16

    jmp  .read_loop

.done:
    ret
