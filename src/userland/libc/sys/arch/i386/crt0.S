.section .text

.global _start
_start:
    movl $0, %ebp
    pushl %ebp
    pushl %ebp
    movl %esp, %ebp

    pushl %eax # argc
    pushl %ebx # argv
    pushl %ecx # envc
    pushl %edx # envp

    call _init

    addl $8, %esp

    call main

    addl $8, %esp

    pushl %eax
    call exit

.size _start, . - _start