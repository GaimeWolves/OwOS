#include <stdint.h>
#include <stddef.h>

#include <multiboot.h>
#include <heap/kmalloc.hpp>
#include <vga/textmode.hpp>
#include <libk/kstdio.hpp>
#include <tests.hpp>

namespace Kernel
{

    extern "C" __attribute__((noreturn)) void entry(uint32_t magic, multiboot_info_t *multiboot_info)
    {
        if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
            LibK::printf_check_msg(false, "Wrong bootloader magic value");

        if (!(multiboot_info->flags & MULTIBOOT_INFO_MEM_MAP))
            LibK::printf_check_msg(false, "No memory map provided");

        VGA::Textmode::init();

        Tests::test_crtx();

        Heap::init();
        Tests::test_heap();

#ifdef _DEBUG
        LibK::printf_debug_msg("Reached end of entry");
#endif

        for (;;)
            ;
    }

} // namespace Kernel