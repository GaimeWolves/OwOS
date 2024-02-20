.section .init, "ax", @progbits
.align 4
.global _init
.type _init, @function
_init:
   pushl %ebp
   movl %esp, %ebp

.section .fini, "ax", @progbits
.align 4
.global _fini
.type _fini, @function
_fini:
   pushl %ebp
   movl %esp, %ebp
