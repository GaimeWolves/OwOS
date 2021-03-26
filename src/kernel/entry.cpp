#include <stdint.h>
#include <stddef.h>

#include <multiboot.h>

static inline uint16_t vga_entry(uint8_t uc, uint8_t color)
{
    return (uint16_t)uc | (uint16_t)color << 8;
}

__attribute__((constructor)) void testCRTX()
{
    char message[] = "HELLO FROM CRTX!";

        for (size_t i = 0; i < sizeof(message) - 1; i++)
        *(((uint16_t *)0xB8000) + i + 80) = vga_entry((uint8_t)message[i], 15);
}

extern "C" __attribute__((noreturn)) void entry(uint32_t magic, multiboot_info_t *multiboot_info)
{
    char message[] = "HELLO WORLD!";
    char wrong_magic_error[] = "WRONG MAGIC";
    char no_mem_error[] = "NO MEM MAP";

    if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
    {
        for (size_t i = 0; i < sizeof(wrong_magic_error) - 1; i++)
            *(((uint16_t *)0xB8000) + i) = vga_entry((uint8_t)wrong_magic_error[i], 15);
    }

    if (!(multiboot_info->flags & MULTIBOOT_INFO_MEM_MAP))
    {
        for (size_t i = 0; i < sizeof(no_mem_error) - 1; i++)
            *(((uint16_t *)0xB8000) + i) = vga_entry((uint8_t)no_mem_error[i], 15);
    }

    for (size_t i = 0; i < sizeof(message) - 1; i++)
        *(((uint16_t *)0xB8000) + i) = vga_entry((uint8_t)message[i], 15);

    for (;;)
        ;
}