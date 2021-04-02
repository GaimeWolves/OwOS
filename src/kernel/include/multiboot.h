#ifndef _MULTIBOOT_H
#define _MULTIBOOT_H 1

#ifndef ASM_FILE
#include <stdint.h>
#endif

// Magic numbers
#define MUTLIBOOT_HEADER_MAGIC 0x1BADB002
#define MULTIBOOT_BOOTLOADER_MAGIC 0x2BADB002

// Alignment info
#define MULTIBOOT_HEADER_ALIGN 4
#define MULTIBOOT_MODULE_ALIGN 4096
#define MULTIBOOT_INFO_ALIGN 4

// OS flags
#define MULTIBOOT_PAGE_ALIGN 0x00000001  // Request 4KiB aligned modules
#define MULTIBOOT_MEMORY_INFO 0x00000002 // Request memory info
#define MULTIBOOT_VIDEO_MODE 0x00000004  // Request video info
#define MULTIBOOT_AOUT_KLUDGE 0x00010000 // Use address fields in header

// Bootloader flags
#define MULTIBOOT_INFO_MEMORY 0x0000001           // Memory info available
#define MULTIBOOT_INFO_BOOTDEVICE 0x0000002       // Boot device set
#define MULTIBOOT_INFO_CMDLINE 0x0000004          // Command line defined
#define MULTIBOOT_INFO_MODULES 0x0000008          // Modules loaded
#define MULTIBOOT_INFO_AOUT_SYMS 0x0000010        // Symbol table available
#define MULTIBOOT_INFO_ELF_SHEADER 0x0000020      // ELF section header available
#define MULTIBOOT_INFO_MEM_MAP 0x0000040          // Full memory map available
#define MULTIBOOT_INFO_DRIVE_INFO 0x0000080       // Drive info available
#define MULTIBOOT_INFO_CONFIG_TABLE 0x0000100     // Config table available
#define MULTIBOOT_INFO_BOOTLOADER_NAME 0x0000200  // Bootloader name set
#define MULTIBOOT_INFO_APM_TABLE 0x0000400        // APM table available
#define MULTIBOOT_INFO_VBE_INFO 0x0000800         // Video information available
#define MULTIBOOT_INFO_FRAMEBUFFER_INFO 0x0001000

#define MULTIBOOT_MEMORY_AVAILABLE 1
#define MULTIBOOT_MEMORY_RESERVED 2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE 3
#define MULTIBOOT_MEMORY_NVS 4
#define MULTIBOOT_MEMORY_BADRAM 5

#ifndef ASM_FILE

// Holds the a.out symbol table information
typedef struct multiboot_aout_symbol_table_t
{
	uint32_t tabsize;
	uint32_t strsize;
	uint32_t addr;
	uint32_t reserved;
} __attribute__((packed)) multiboot_aout_symbol_table_t;

// Holds the ELF section header table information
typedef struct multiboot_elf_section_header_table_t
{
	uint32_t num;
	uint32_t size;
	uint32_t addr;
	uint32_t shndx;
} __attribute__((packed)) multiboot_elf_section_header_table_t;

// Holds the complete multiboot information structure provided by the bootloader
typedef struct multiboot_info_t
{
	// Multiboot info flags
	uint32_t flags;

	// Available memory
	uint32_t memory_lo;
	uint32_t memory_hi;

	// Boot partition
	uint32_t boot_device;

	// Kernel command line
	uint32_t cmdline;

	// Boot-Module information
	uint32_t mods_count;
	uint32_t mods_addr;

	// Symbol table information / Section header information
	union
	{
		multiboot_aout_symbol_table_t aout_syms;
		multiboot_elf_section_header_table_t elf_sec;
	};

	// Memory map information
	uint32_t mmap_length;
	uint32_t mmap_addr;

	// Drive information
	uint32_t drives_length;
	uint32_t drives_addr;

	// ROM configuration table
	uint32_t config_table;

	// Bootloader name
	uint32_t bootloader_name;

	// APM table
	uint32_t apm_table;

	// Video information
	uint32_t vbe_ctrl_info;
	uint32_t vbe_mode_info;
	uint16_t vbe_io_seg;
	uint32_t vbe_io_off;
	uint16_t vbe_io_len;

	uint64_t framebuffer_addr;
	uint32_t framebuffer_pitch;
	uint32_t framebuffer_width;
	uint32_t framebuffer_height;
	uint8_t framebuffer_bpp;
	uint8_t framebuffer_type;

	union
	{
		struct
		{
			uint32_t framebuffer_palette_addr;
			uint16_t framebuffer_palette_num_colors;
		};
		struct
		{
			uint8_t framebuffer_red_field_position;
			uint8_t framebuffer_red_mask_size;
			uint8_t framebuffer_green_field_position;
			uint8_t framebuffer_green_mask_size;
			uint8_t framebuffer_blue_field_position;
			uint8_t framebuffer_blue_mask_size;
		};
	};
} __attribute__((packed)) multiboot_info_t;

typedef struct multiboot_mmap_entry_t
{
	uint32_t size;
	uint64_t addr;
	uint64_t len;
	uint32_t type;
} __attribute__((packed)) multiboot_mmap_entry_t;

#endif

#endif // _MULTIBOOT_H