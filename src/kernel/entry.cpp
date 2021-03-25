#include <stdint.h>
#include <stddef.h>

#include <multiboot.h>

static inline uint16_t vga_entry(uint8_t uc, uint8_t color)
{
    return (uint16_t)uc | (uint16_t)color << 8;
}

extern "C" __attribute__((noreturn)) void entry(uint32_t magic, multiboot_info_t *multiboot_info)
{
    char message[] = "HELLO WORLD!";
    char wrong_magic_error[] = "WRONG MAGIC";
    char no_mem_error[] = "NO MEM MAP";

    if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
    {
        for (size_t i = 0; i < 11; i++)
            *(((uint16_t *)0xB8000) + i) = vga_entry((uint8_t)wrong_magic_error[i], 15);
    }

    if (!(multiboot_info->flags & MULTIBOOT_INFO_MEM_MAP))
    {
        for (size_t i = 0; i < 10; i++)
            *(((uint16_t *)0xB8000) + i) = vga_entry((uint8_t)no_mem_error[i], 15);
    }

    for (size_t i = 0; i < 12; i++)
        *(((uint16_t *)0xB8000) + i) = vga_entry((uint8_t)message[i], 15);

    for (;;)
        ;
}