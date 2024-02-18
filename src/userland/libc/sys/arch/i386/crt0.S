.section .text

.global _start
_start:
    movl $0, %ebp
    pushl %ebp
    pushl %ebp
    movl %esp, %ebp

    call _init

    movl 0x0c(%ebp), %eax # argc
    movl 0x10(%ebp), %ebx # argv

    push %ebx
    push %eax

    call __libc_init

    call main

    addl $8, %esp

    pushl %eax
    call exit

.size _start, . - _start
