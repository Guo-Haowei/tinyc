; ----------------------------------------------------------------------------------------
; nasm -felf64 test.asm && gcc --static test.o
; ----------------------------------------------------------------------------------------
    section .text
    global main
    extern printf
main:
    push rdi ; save first param int argc
    push rsi ; save second param const char** argv

    sub rsp, 8 ; rsp must be 16-byte aligned
    mov rdi, format ; first param
    mov rsi, [rsi] ; first param
    xor rax, rax
    call printf
    add rsp, 8 ; restore rsp

    pop rsi
    pop rdi

    ; return 0
    ; xor rax, rax
    ret

    section .data
format:
    db  "program is %s",10,0
