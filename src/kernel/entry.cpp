#include <stdint.h>
#include <stddef.h>

#include <multiboot.h>
#include <memory/MemoryManager.hpp>
#include <vga/textmode.hpp>
#include <tests.hpp>

#include <libk/kmalloc.hpp>
#include <libk/kstdio.hpp>
#include <libk/kstring.hpp>

namespace Kernel
{
	extern "C" __attribute__((noreturn)) void entry(uint32_t magic, multiboot_info_t *multiboot_info)
	{
		assert(magic == MULTIBOOT_BOOTLOADER_MAGIC);
		assert(multiboot_info);

		VGA::Textmode::init();

		Tests::test_crtx();
		Tests::test_heap();
		Tests::test_printf();

#ifdef _DEBUG
		LibK::printf_debug_msg("Reached end of entry");
#endif

		for (;;)
			;
	}
} // namespace Kernel