.section .text.ap_boot

.extern ap_entry
.type ap_entry, @function

.global _start_ap_boot
.set _start_ap_boot, .

.global boot_ap
.type boot_ap, @function
boot_ap:
.code16
        cli
        cld
        xorw %ax, %ax
        movw %ax, %ds

        // load initial GDT
        lgdt (ap_early_gdtr - boot_ap + 0x8000)

        // enable PM
        movl %cr0, %eax
        orl $1, %eax
        movl %eax, %cr0

        ljmpl $8, $(.boot_ap_32 - boot_ap + 0x8000)

    .boot_ap_32:
    .code32
        mov $0x10, %ax
        mov %ax, %ss
        mov %ax, %ds
        mov %ax, %es
        mov %ax, %fs
        mov %ax, %gs

        movl $0x8000, %ebp

        movl (ap_bsp_cr3 - boot_ap)(%ebp), %eax
        movl %eax, %cr3

        movl %cr0, %eax
        orl $0x80000000, %eax
        movl %eax, %cr0

        ljmpl $8, $.boot_ap_32_2
    .boot_ap_32_2:
        movl %cr3, %eax
        movl %eax, %cr3

        movl (ap_bsp_gdtr - boot_ap)(%ebp), %eax
        lgdt (%eax)

        movl (ap_bsp_idtr - boot_ap)(%ebp), %eax
        lidt (%eax)

        movl (ap_cpu_id - boot_ap)(%ebp), %ebx
        xorl %eax, %eax
        incl %eax
        lock xaddl %eax, (%ebx)
        movl %eax, %esi

        movl (ap_stacks - boot_ap)(%ebp, %eax, 4), %esp

    .wait_for_bsp:
        pause
        movl (ap_do_continue - boot_ap)(%ebp), %eax
        cmpl $1, (%eax)
        jnz .wait_for_bsp

        xor %ebp, %ebp

        incl %esi
        push %esi
        push .loop
        ljmpl $8, $ap_entry

    .loop:
        cli
        hlt
        jmp .loop

.align 16
ap_early_gdt:
    // null
    .8byte 0

    // code
    .4byte 0x0000FFFF
    .4byte 0x00CF9A00

    // data
    .4byte 0x0000FFFF
    .4byte 0x00CF9200
ap_early_gdtr:
    .2byte ap_early_gdtr - ap_early_gdt - 1
    .4byte (ap_early_gdt - boot_ap) + 0x8000
.align 4
.global ap_bsp_cr3
ap_bsp_cr3:
    .4byte 0
.align 4
.global ap_bsp_idtr
ap_bsp_idtr:
    .4byte 0
.align 4
.global ap_bsp_gdtr
ap_bsp_gdtr:
    .4byte 0
.align 4
.global ap_cpu_id
ap_cpu_id:
    .4byte 0
.align 4
.global ap_do_continue
ap_do_continue:
    .4byte 0
.align 4
.global ap_code_virt_addr
ap_code_virt_addr:
    .4byte 0
.align 4
.global ap_stacks
ap_stacks:

.global _end_ap_boot
.set _end_ap_boot, .