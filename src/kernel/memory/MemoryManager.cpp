#include <memory/MemoryManager.hpp>

#include <arch/processor.hpp>
#include <memory/MultibootMap.hpp>

#include <libk/kcmalloc.hpp>
#include <libk/kcstring.hpp>
#include <libk/kmath.hpp>

#define PAGE_COUNT     1024
#define PAGE_SIZE      (PAGE_COUNT * sizeof(uint32_t))
#define PAGE_SIZE_HUGE (PAGE_SIZE * PAGE_COUNT)
#define DIRECTORY_SIZE PAGE_SIZE_HUGE

#define OFFSET_BITS    12
#define TABLE_BITS     10
#define DIRECTORY_BITS 10

#define OFFSET_MASK    0xFFF
#define TABLE_MASK     0x3FF000
#define DIRECTORY_MASK 0xFFC00000

#define PAGE_DIRECTORY_ADDR   0xFFFFF000
#define PAGE_TABLE_ARRAY_ADDR 0xFFC00000

// For now all memory mapped IO (MMIO) is just mapped with this offset
// TODO: Dynamically allocate memory for kernel and later user space
#define MMIO_START_ADDR 0xD0000000

extern "C"
{
	extern uintptr_t _virtual_addr;
	extern uintptr_t _physical_addr;
	extern uintptr_t _kernel_start;
	extern uintptr_t _kernel_end;
}

namespace Kernel::Memory
{
	inline static constexpr size_t to_page_address(uintptr_t physical_address);
	inline static constexpr size_t to_directory_address(uintptr_t physical_address);

	inline static constexpr size_t get_pd_index(uintptr_t virtual_address);
	inline static constexpr size_t get_pt_index(uintptr_t virtual_address);
	inline static constexpr size_t get_offset(uintptr_t virtual_address);

	inline static page_directory_t &get_page_directory();
	inline static page_table_t &get_page_table(size_t pt_index);

	inline static uintptr_t as_physical(uintptr_t virtual_address);

	inline static constexpr size_t to_page_address(uintptr_t physical_address)
	{
		return physical_address & (TABLE_MASK | DIRECTORY_MASK);
	}

	inline static constexpr size_t to_directory_address(uintptr_t physical_address)
	{
		return physical_address & DIRECTORY_MASK;
	}

	inline static constexpr size_t get_pd_index(uintptr_t virtual_address)
	{
		return (virtual_address & DIRECTORY_MASK) >> (TABLE_BITS + OFFSET_BITS);
	}

	inline static constexpr size_t get_pt_index(uintptr_t virtual_address)
	{
		return (virtual_address & TABLE_MASK) >> OFFSET_BITS;
	}

	inline static constexpr size_t get_offset(uintptr_t virtual_address)
	{
		return virtual_address & OFFSET_MASK;
	}

	inline static page_directory_t &get_page_directory()
	{
		return *(page_directory_t *)PAGE_DIRECTORY_ADDR;
	}

	inline static page_table_t &get_page_table(size_t pt_index)
	{
		return *(page_table_t *)(PAGE_TABLE_ARRAY_ADDR + PAGE_SIZE * pt_index);
	}

	inline static uintptr_t as_physical(uintptr_t virtual_address)
	{
		uintptr_t pd_index = get_pd_index(virtual_address);
		uintptr_t pt_index = get_pt_index(virtual_address);

		assert(get_page_directory()[pd_index].present);

		page_table_t &page_table = get_page_table(pd_index);
		assert(page_table[pt_index].present);

		return (uintptr_t)page_table[pt_index].page() + get_offset(virtual_address);
	}

	MemoryManager::MemoryManager()
	{
		m_kernel_pd = (page_directory_t *)kcalloc(PAGE_SIZE, PAGE_SIZE);
		m_current_pd = m_kernel_pd;

		map_page_directory();
		m_kernel_mapping_table = m_current_mapping_table;

		map_kernel();
		Processor::load_page_directory((uintptr_t)m_current_pd - (uintptr_t)&_virtual_addr);
	}

	void MemoryManager::init(multiboot_info_t *&multiboot_info)
	{
		static bool isInitialized = false;

		if (!isInitialized)
		{
			preserve_multiboot_info(multiboot_info);
			m_memory_map.load_map(multiboot_info);

			isInitialized = true;
		}
	}

	// Initialize the map of last PDE to the page directory itself so we can use
	// addresses 0xFFC00000 - 0xFFFFF000 as a lookup table for physical adresses
	void MemoryManager::map_page_directory()
	{
		page_directory_t &page_directory = *m_current_pd;

		page_table_t &page_table = *(page_table_t *)kcalloc(PAGE_SIZE, PAGE_SIZE);
		uintptr_t pt_address = (uintptr_t)&page_table - (uintptr_t)&_virtual_addr;
		uintptr_t pd_address = (uintptr_t)&page_directory - (uintptr_t)&_virtual_addr;

		m_current_mapping_table = &page_table;

		page_directory[1023] = page_directory_entry_t{
		    .present = true,
		    .writeable = true,
		    .user = false,
		    .write_through = false,
		    .cache_disable = false,
		    .accessed = false,
		    .page_size = false,
		    .page_table_address = pt_address >> 12,
		};

		page_table[1023] = page_table_entry_t{
		    .present = true,
		    .writeable = true,
		    .user = false,
		    .write_through = false,
		    .cache_disable = false,
		    .accessed = false,
		    .dirty = false,
		    .global = false,
		    .page_address = pd_address >> 12,
		};
	}

	// Manually map the kernel as we cannot have the lookup table loaded
	// at this stage
	void MemoryManager::map_kernel()
	{
		uintptr_t p_address = (uintptr_t)&_physical_addr;
		size_t size = (uintptr_t)&_kernel_end - (uintptr_t)&_kernel_start;
		uintptr_t v_address = (uintptr_t)&_kernel_start;

		uintptr_t page_limit = (p_address + size + 1 + PAGE_SIZE - 1) / PAGE_SIZE * PAGE_SIZE;

		for (; p_address < page_limit; p_address += PAGE_SIZE, v_address += PAGE_SIZE)
		{
			size_t pd_index = get_pd_index(v_address);
			page_directory_t &page_directory = *m_current_pd;

			if (!page_directory[pd_index].value())
			{
				page_table_t &page_table = *(page_table_t *)kcalloc(PAGE_SIZE, PAGE_SIZE);
				uintptr_t address = (uintptr_t)&page_table - (uintptr_t)&_virtual_addr;

				page_directory[pd_index] = page_directory_entry_t{
				    .present = true,
				    .writeable = true,
				    .user = false,
				    .write_through = false,
				    .cache_disable = false,
				    .accessed = false,
				    .page_size = false,
				    .page_table_address = address >> 12,
				};

				assert(page_directory[1023].present);

				page_table_t &mapping_table = *m_current_mapping_table;
				mapping_table[pd_index] = page_table_entry_t{
				    .present = true,
				    .writeable = true,
				    .user = false,
				    .write_through = false,
				    .cache_disable = false,
				    .accessed = false,
				    .dirty = false,
				    .global = false,
				    .page_address = address >> 12,
				};
			}

			page_table_t &page_table = *(page_table_t *)((uintptr_t)page_directory[pd_index].table() + (uintptr_t)&_virtual_addr);

			size_t pt_index = get_pt_index(v_address);
			uintptr_t page_address = to_page_address(p_address);

			page_table[pt_index] = page_table_entry_t{
			    .present = true,
			    .writeable = true,
			    .user = false,
			    .write_through = false,
			    .cache_disable = false,
			    .accessed = false,
			    .dirty = false,
			    .global = false,
			    .page_address = page_address >> 12,
			};
		}
	}

	// Identity map the given multiboot structures, then allocate the necessary memory on the heap
	// and move the data there. Finally unmap the old multiboot structures
	void MemoryManager::preserve_multiboot_info(multiboot_info_t *&multiboot_info)
	{
		// Preserve multiboot header
		assert(multiboot_info);
		map_range((uintptr_t)multiboot_info, sizeof(multiboot_info_t), (uintptr_t)multiboot_info, false, true, false, true);
		multiboot_info_t *new_multiboot_info = (multiboot_info_t *)kmalloc(sizeof(multiboot_info_t));
		memmove(new_multiboot_info, multiboot_info, sizeof(multiboot_info_t));
		unmap_range((uintptr_t)multiboot_info, sizeof(multiboot_info_t), true);
		multiboot_info = new_multiboot_info;

		// Preserve memory map
		assert(multiboot_info->flags & MULTIBOOT_INFO_MEM_MAP);
		map_range(multiboot_info->mmap_addr, multiboot_info->mmap_length, multiboot_info->mmap_addr, false, true, false, true);
		multiboot_mmap_entry_t *new_mmap_addr = (multiboot_mmap_entry_t *)kmalloc(multiboot_info->mmap_length);
		memmove(new_mmap_addr, (void *)multiboot_info->mmap_addr, multiboot_info->mmap_length);
		unmap_range(multiboot_info->mmap_addr, multiboot_info->mmap_length, true);
		multiboot_info->mmap_addr = (uint32_t)new_mmap_addr;
	}

	page_directory_entry_t MemoryManager::create_pde(size_t pd_index, bool is_user, bool is_writeable, bool disable_cache)
	{
		page_table_t *table = (page_table_t *)kcalloc(PAGE_SIZE, PAGE_SIZE);
		uintptr_t address = as_physical((uintptr_t)table);

		auto &mapping_table = *m_current_mapping_table;
		mapping_table[pd_index] = create_pte(address, get_page_directory()[1023], false, true, false);

		return page_directory_entry_t{
		    .present = true,
		    .writeable = is_writeable,
		    .user = is_user,
		    .write_through = false,
		    .cache_disable = disable_cache,
		    .accessed = false,
		    .page_size = false,
		    .page_table_address = (uint32_t)address >> 12,
		};
	}

	page_table_entry_t MemoryManager::create_pte(uintptr_t page_address, page_directory_entry_t &pde, bool is_user, bool is_writeable, bool disable_cache)
	{
		// User pages are only allowed in user page directories
		assert(!is_user || (is_user && !pde.user));

		return page_table_entry_t{
		    .present = true,
		    .writeable = is_writeable,
		    .user = is_user,
		    .write_through = false,
		    .cache_disable = disable_cache,
		    .accessed = false,
		    .dirty = false,
		    .global = false,
		    .page_address = page_address >> 12,
		};
	}

	void MemoryManager::map(uintptr_t physical_address, uintptr_t virtual_address, bool is_user, bool is_writeable, bool disable_cache, bool invalidate)
	{
		size_t pd_index = get_pd_index(virtual_address);
		page_directory_t &page_directory = get_page_directory();

		if (!page_directory[pd_index].value())
			page_directory[pd_index] = create_pde(pd_index, is_user, is_writeable, disable_cache);

		assert(page_directory[pd_index].present);

		size_t pt_index = get_pt_index(virtual_address);
		page_table_t &page_table = get_page_table(pd_index);

		assert(!page_table[pt_index].present);

		uintptr_t page_address = to_page_address(physical_address);

		page_table[pt_index] = create_pte(page_address, page_directory[pd_index], is_user, is_writeable, disable_cache);

		if (invalidate)
			Processor::invalidate_address(physical_address);
	}

	void MemoryManager::map_range(uintptr_t physical_address, size_t size, uintptr_t virtual_address, bool is_user, bool is_writeable, bool disable_cache, bool invalidate)
	{
		uintptr_t p_address = physical_address;
		uintptr_t v_address = virtual_address;

		uintptr_t page_limit = (p_address + size + 1 + PAGE_SIZE - 1) / PAGE_SIZE * PAGE_SIZE;

		for (; p_address < page_limit; p_address += PAGE_SIZE, v_address += PAGE_SIZE)
			map(p_address, v_address, is_user, is_writeable, disable_cache, false);

		if (invalidate)
			Processor::flush_page_directory();
	}

	void MemoryManager::unmap(uintptr_t virtual_address, bool invalidate)
	{
		static page_table_entry_t null_entry{};

		size_t pd_index = get_pd_index(virtual_address);
		auto &page_directory = get_page_directory();
		assert(page_directory[pd_index].present);

		size_t pt_index = get_pt_index(virtual_address);
		auto &page_table = get_page_table(pd_index);
		assert(page_table[pt_index].present);

		uintptr_t p_address = (uintptr_t)page_table[pt_index].page();

		page_table[pt_index] = null_entry;

		if (invalidate)
			Processor::invalidate_address(p_address);
	}

	void MemoryManager::unmap_range(uintptr_t virtual_address, size_t size, bool invalidate)
	{
		uintptr_t v_address = virtual_address;

		for (; v_address <= virtual_address + size; v_address += PAGE_SIZE)
			unmap(v_address, false);

		if (invalidate)
			Processor::flush_page_directory();
	}

	void *MemoryManager::map_mmio_region(uintptr_t physical_addr, size_t size)
	{
		map_range(physical_addr, size, physical_addr + MMIO_START_ADDR, false, true, true, true);
		return (void *)(physical_addr + MMIO_START_ADDR);
	}

	void MemoryManager::unmap_region(uintptr_t virtual_addr, size_t size)
	{
		unmap_range(virtual_addr, size, true);
	}
}; // namespace Kernel::Memory