#define ASM_FILE 1
#include <multiboot.h>

.set FLAGS, (MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO)
.set MAGIC, MUTLIBOOT_HEADER_MAGIC
.set CHECKSUM, -(MAGIC + FLAGS)

.section .multiboot
    .align 4
    .long MAGIC
    .long FLAGS
    .long CHECKSUM

.section .bss
    .align 16
    stack_begin:
    .skip 16384
    stack_end:

.section .text
    .global _entry
    .type _entry, @function
    _entry:
        movl $stack_end, %esp
        movl %esp, %ebp

        pushl $0
        popf

        push %ebx
        push %eax

        call entry

        cli
        hlt

    loop:
        jmp loop

    .size _entry, . - _entry