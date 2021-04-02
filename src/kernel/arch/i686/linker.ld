ENTRY(_entry)

SECTIONS
{
	_virtual_addr = 0xC0000000;
	_physical_addr = 0x00100000;

	. = _virtual_addr + _physical_addr;

	_kernel_start = .;

	.text BLOCK(4K) : AT (ADDR (.text) - _virtual_addr)
	{
		*(.multiboot.data)
		*(.multiboot.text)
		*(.text)
	}

	.rodata BLOCK(4K) : AT (ADDR (.rodata) - _virtual_addr)
	{
		_start_heap_ctors = .;
		*libkernel_heap.a:*(.ctors)
		_end_heap_ctors = .;

		_start_ctors = .;
		*(.ctors)
		_end_ctors = .;

		_start_dtors = .;
		*(.ctors)
		_end_dtors = .;

		*(.rodata)
	}

	.data BLOCK(4K) : AT (ADDR (.data) - _virtual_addr)
	{
		*(.data)
	}

	.bss BLOCK(4K) : AT (ADDR (.bss) - _virtual_addr)
	{
		*(COMMON)
		*(.bss)

		. = ALIGN(4K);
		*(.heap)
	}

	_kernel_end = .;
}