.section .rodata
.global __font_start
.global __font_end

.align 8
__font_start:
    .incbin "@file@"
__font_end:
