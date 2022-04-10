#pragma once

#include <stddef.h>
#include <stdint.h>

#include <common_attributes.h>

#define PAGE_SIZE 4096

namespace Kernel::Memory::Arch
{
	typedef struct page_table_entry_t
	{
		uint32_t present : 1;
		uint32_t writeable : 1;
		uint32_t user : 1;
		uint32_t write_through : 1;
		uint32_t cache_disable : 1;
		uint32_t accessed : 1;
		uint32_t dirty : 1,
		    : 1; // Always zero
		uint32_t global : 1,
		    : 3; // May be used for OS-specific things
		uint32_t page_address : 20;

		inline void *page() { return (void *)(page_address << 12); }
		inline uint32_t value() { return *(uint32_t *)this; }
	} __packed page_table_entry_t;

	typedef struct page_table_t
	{
		page_table_entry_t pages[1024];

		page_table_entry_t &operator[](int i) { return pages[i]; }
	} __packed_align(PAGE_SIZE) page_table_t;

	typedef struct page_directory_entry_t
	{
		uint32_t present : 1;
		uint32_t writeable : 1;
		uint32_t user : 1;
		uint32_t write_through : 1;
		uint32_t cache_disable : 1;
		uint32_t accessed : 1,
		    : 1; // Always zero
		uint32_t page_size : 1,
		    : 1, // Ignored
		    : 3; // May be used for OS-specific things
		uint32_t page_table_address : 20;

		inline page_table_t *table() { return (page_table_t *)(page_table_address << 12); }
		inline uint32_t value() { return *(uint32_t *)this; }
	} __packed page_directory_entry_t;

	typedef struct page_directory_t
	{
		page_directory_entry_t tables[1024];

		inline page_directory_entry_t &operator[](int i) { return tables[i]; }
	} __packed_align(PAGE_SIZE) page_directory_t;

	struct paging_space_t
	{
		uintptr_t physical_pd_address;
		page_directory_t *page_directory;
		page_table_t *mapping_table;
	};

	Arch::paging_space_t get_kernel_paging_space();
} // namespace Kernel::Memory::Arch