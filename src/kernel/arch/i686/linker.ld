ENTRY(_entry)

PHDRS
{
  text PT_LOAD FLAGS(0x05) ;
  rodata PT_LOAD FLAGS(0x04) ;
  data PT_LOAD FLAGS(0x06) ;
  bss PT_LOAD FLAGS(0x06) ;
  kernel_symbols PT_LOAD FLAGS(0x04) ;
}

SECTIONS
{
	_virtual_addr = 0xC0000000;
	_physical_addr = 0x00100000;

	. = _virtual_addr + _physical_addr;

	_kernel_start = .;

	.text BLOCK(4K) : AT (ADDR (.text) - _virtual_addr)
	{
	    _start_text = .;
		*(.multiboot.data)
		*(.multiboot.text)
		*(.text)
	} :text

	.rodata BLOCK(4K) : AT (ADDR (.rodata) - _virtual_addr)
	{
	    _end_text = .;

		_start_heap_ctors = .;
		*libkernel_heap.a:*(.ctors)
		_end_heap_ctors = .;

		_start_ctors = .;
		*(.ctors)
		_end_ctors = .;

		_start_dtors = .;
		*(.dtors)
		_end_dtors = .;

		_start_init_array = .;
		*(.init_array)
		_end_init_array = .;

		*(.rodata)
	} :rodata

	.data BLOCK(4K) : AT (ADDR (.data) - _virtual_addr)
	{
		*(.data)
	} :data

	.bss BLOCK(4K) : AT (ADDR (.bss) - _virtual_addr)
	{
		*(COMMON)
		*(.bss)

		. = ALIGN(4K);
		*(.heap)
	} :bss

	.kernel_symbols BLOCK(4K) : AT (ADDR (.kernel_symbols) - _virtual_addr)
	{
		_start_kernel_symbols = .;
		KEEP(*(.kernel_symbols))
		_end_kernel_symbols = .;
	} :kernel_symbols
	
	_kernel_end = .;
}
