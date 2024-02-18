#define ASM_FILE 1
#include <multiboot.h>

.set FLAGS, (MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO | MULTIBOOT_VIDEO_MODE)
.set MAGIC, MUTLIBOOT_HEADER_MAGIC
.set CHECKSUM, -(MAGIC + FLAGS)
.set virt_addr, 0xC0000000

.section .multiboot.data
	.align 4
	.long MAGIC
	.long FLAGS
	.long CHECKSUM

    // MULTIBOOT_MEMORY_INFO
    .long   0
    .long   0
    .long   0
    .long   0
    .long   0

    // MULTIBOOT_VIDEO_MODE
    .long 0
    .long 1024
    .long 768
    .long 32

.section .bss
	multiboot_magic: .4byte 0
	multiboot_addr: .4byte 0

	.align 16
	stack_begin:
		.skip 16384
	stack_end:

	.align 4096
	bootstrap_page_directory:
		.skip 4096
	bootstrap_page_table_1:
		.skip 4096
	bootstrap_page_table_2: # Add more tables if necessary
		.skip 4096

.section .multiboot.text
	.global _entry
	.type _entry, @function
	_entry:
			# Save passed arguments in %eax and %ebx
			movl %eax, multiboot_magic - virt_addr
			movl %ebx, multiboot_addr - virt_addr

			movl $(bootstrap_page_table_1 - virt_addr), %edi
			movl $0, %esi
			movl $2048, %ecx
		.loop_map_kernel:
			cmpl $(_kernel_end - virt_addr), %esi
			jg .enable_paging

			# Map physical address to virtual address
			# NOTE: all memory is "present, writeable"
			movl %esi, %edx
			or $0x003, %edx
			movl %edx, (%edi)
		.loop_map_kernel_increment:
			addl $4096, %esi
			addl $4, %edi
			loop .loop_map_kernel
		.enable_paging:
			# Both identity map and map to 0xC0000000 the
			# PD0 (from 0x0 - 0x3FFFFF) up to as many as needed
			movl $(bootstrap_page_table_1 - virt_addr + 0x003), bootstrap_page_directory - virt_addr + 0 * 4
			movl $(bootstrap_page_table_1 - virt_addr + 0x003), bootstrap_page_directory - virt_addr + 768 * 4

			# PD1 (from 0x400000 - 0x7FFFFF)
			movl $(bootstrap_page_table_2 - virt_addr + 0x003), bootstrap_page_directory - virt_addr + 1 * 4
			movl $(bootstrap_page_table_2 - virt_addr + 0x003), bootstrap_page_directory - virt_addr + 769 * 4

			# Point cr3 to bootstrap_page_directory
			movl $(bootstrap_page_directory - virt_addr), %ecx
			movl %ecx, %cr3

			# Enable paging and the write-protect bit.
			movl %cr0, %ecx
			orl $0x80010000, %ecx
			movl %ecx, %cr0

			# Jump to higher half with an absolute jump
			lea _call_entry, %ecx
			jmp *%ecx

.section .text
	_call_entry:
			# Unmap the identity mapping as it is now unnecessary.
			movl $0, bootstrap_page_directory + 0 * 4
			movl $0, bootstrap_page_directory + 1 * 4

			# Reload CR3 to force a TLB flush so the changes take effect.
			movl %cr3, %ecx
			movl %ecx, %cr3

			movl $stack_end, %esp
			xor %ebp, %ebp

			push (multiboot_addr)
			push (multiboot_magic)

			call early_entry

			cli
			hlt

		.loop:
			jmp .loop

	.size _call_entry, . - _call_entry
