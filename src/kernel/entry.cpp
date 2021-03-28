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
        Tests::test_crtx();

        if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
            print_check_msg(false, "Wrong bootloader magic value");

        if (!(multiboot_info->flags & MULTIBOOT_INFO_MEM_MAP))
            print_check_msg(false, "No memory map provided");

        VGA::Textmode::init();

        Heap::init();
        Tests::test_heap();

        for (;;)
            ;
    }

} // namespace Kernel