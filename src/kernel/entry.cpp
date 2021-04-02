#include <stdint.h>
#include <stddef.h>

#include <multiboot.h>
#include <heap/kmalloc.hpp>
#include <memory/MemoryManager.hpp>
#include <vga/textmode.hpp>
#include <libk/kstdio.hpp>
#include <libk/kstring.hpp>
#include <tests.hpp>

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