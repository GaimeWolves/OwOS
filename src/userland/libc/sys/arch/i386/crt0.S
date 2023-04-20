.section .text

.global _start
_start:
    movl $0, %ebp
    pushl %ebp
    pushl %ebp
    movl %esp, %ebp

    movl 0x0c(%ebp), %ecx # argc
    movl 0x10(%ebp), %ebx # argv
    movl 0x14(%ebp), %eax # envp

    pushl %eax
    pushl %ebx
    pushl %ecx

    call _init

    call __libc_init

    call main

    addl $8, %esp

    pushl %eax
    call exit

.size _start, . - _start