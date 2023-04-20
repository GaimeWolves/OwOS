.section .rodata
.global __font_start
.global __font_end

__font_start:
    .incbin "@file@"
__font_end: