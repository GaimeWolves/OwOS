#include <arch/memory.hpp>

#include <arch/Processor.hpp>
#include <common_attributes.h>
#include <memory/PhysicalMemoryManager.hpp>

#include <libk/kcassert.hpp>
#include <libk/kcmalloc.hpp>
#include <libk/kcstring.hpp>
#include <libk/kfunctional.hpp>
#include <libk/kmath.hpp>
#include <libk/kvector.hpp>

#define PAGE_COUNT     1024
#define PAGE_SIZE_HUGE (PAGE_SIZE * PAGE_COUNT)
#define TABLE_SIZE     PAGE_SIZE_HUGE

#define OFFSET_BITS    12
#define TABLE_BITS     10
#define DIRECTORY_BITS 10

#define OFFSET_MASK    0xFFF
#define TABLE_MASK     0x3FF000
#define DIRECTORY_MASK 0xFFC00000

#define PAGE_DIRECTORY_ADDR   0xFFFFF000
#define PAGE_TABLE_ARRAY_ADDR 0xFFC00000

extern "C"
{
	extern uintptr_t _virtual_addr;
	extern uintptr_t _physical_addr;
	extern uintptr_t _kernel_start;
	extern uintptr_t _kernel_end;
}

namespace Kernel::Memory::Arch
{
	inline static constexpr size_t to_page_address(uintptr_t phys_addr);
	inline static constexpr size_t to_directory_address(uintptr_t phys_addr);

	inline static constexpr size_t get_pd_index(uintptr_t virt_addr);
	inline static constexpr size_t get_pt_index(uintptr_t virt_addr);
	inline static constexpr size_t get_offset(uintptr_t virt_addr);

	inline static page_directory_t &get_page_directory();
	inline static page_table_t &get_page_table(size_t pd_index);

	inline static page_directory_entry_t raw_create_pde(uintptr_t table_addr, bool is_user, bool is_writeable, bool disable_cache);
	inline static page_table_entry_t raw_create_pte(uintptr_t page_addr, bool is_user, bool is_writeable, bool disable_cache);

	static page_directory_entry_t create_pde(size_t pd_index, paging_space_t &memory_space, bool is_user, bool is_writeable, bool disable_cache);
	static page_table_entry_t create_pte(uintptr_t page_address, page_directory_entry_t &pde, bool is_user, bool is_writeable, bool disable_cache);

	static void for_page_in_range(uintptr_t virt_addr, size_t size, LibK::function<void(uintptr_t)> callback);

	static paging_space_t s_kernel_paging_space{};

	inline static constexpr size_t to_page_address(uintptr_t phys_addr)
	{
		return phys_addr & (TABLE_MASK | DIRECTORY_MASK);
	}

	inline static constexpr size_t to_directory_address(uintptr_t phys_addr)
	{
		return phys_addr & DIRECTORY_MASK;
	}

	inline static constexpr size_t get_pd_index(uintptr_t virt_addr)
	{
		return (virt_addr & DIRECTORY_MASK) >> (TABLE_BITS + OFFSET_BITS);
	}

	inline static constexpr size_t get_pt_index(uintptr_t virt_addr)
	{
		return (virt_addr & TABLE_MASK) >> OFFSET_BITS;
	}

	inline static constexpr size_t get_offset(uintptr_t virt_addr)
	{
		return virt_addr & OFFSET_MASK;
	}

	inline static page_directory_t &get_page_directory()
	{
		return *(page_directory_t *)PAGE_DIRECTORY_ADDR;
	}

	inline static page_table_t &get_page_table(size_t pd_index)
	{
		return *(page_table_t *)(PAGE_TABLE_ARRAY_ADDR + PAGE_SIZE * pd_index);
	}

	inline static page_directory_entry_t raw_create_pde(uintptr_t table_addr, bool is_user, bool is_writeable, bool disable_cache)
	{
		return page_directory_entry_t{
		    .present = true,
		    .writeable = is_writeable,
		    .user = is_user,
		    .write_through = false,
		    .cache_disable = disable_cache,
		    .accessed = false,
		    .page_size = false,
		    .page_table_address = (uint32_t)table_addr >> OFFSET_BITS,
		};
	}

	inline static page_table_entry_t raw_create_pte(uintptr_t page_addr, bool is_user, bool is_writeable, bool disable_cache)
	{
		return page_table_entry_t{
		    .present = true,
		    .writeable = is_writeable,
		    .user = is_user,
		    .write_through = false,
		    .cache_disable = disable_cache,
		    .accessed = false,
		    .dirty = false,
		    .global = false,
		    .page_address = (uint32_t)page_addr >> OFFSET_BITS,
		};
	}

	static page_directory_entry_t create_pde(size_t pd_index, paging_space_t &memory_space, bool is_user, bool is_writeable, bool disable_cache)
	{
		uintptr_t table_addr = (uintptr_t)PhysicalMemoryManager::instance().alloc(PAGE_SIZE);

		auto &mapping_table = *memory_space.mapping_table;
		mapping_table[pd_index] = raw_create_pte(table_addr, is_user, is_writeable, disable_cache);

		// Clear it immediately to prevent bugs when we use this page table
		CPU::Processor::invalidate_address((uintptr_t)&get_page_table(pd_index));
		memset(&get_page_table(pd_index), 0, PAGE_SIZE);

		return raw_create_pde(table_addr, is_user, is_writeable, disable_cache);
	}

	static page_table_entry_t create_pte(uintptr_t page_address, page_directory_entry_t &pde, bool is_user, bool is_writeable, bool disable_cache)
	{
		// User pages are only allowed in user page directories
		assert(!is_user || (is_user && pde.user));

		return raw_create_pte(page_address, is_user, is_writeable, disable_cache);
	}

	static void for_page_in_range(uintptr_t virt_addr, size_t size, LibK::function<void(uintptr_t)> callback)
	{
		uintptr_t page_limit = LibK::round_up_to_multiple<uintptr_t>(virt_addr + size, PAGE_SIZE);

		for (; virt_addr < page_limit; virt_addr += PAGE_SIZE)
			callback(virt_addr);
	}

	uintptr_t as_physical(uintptr_t virt_addr)
	{
		uintptr_t pd_index = get_pd_index(virt_addr);
		uintptr_t pt_index = get_pt_index(virt_addr);

		assert(get_page_directory()[pd_index].present);

		auto &page_table = get_page_table(pd_index);
		assert(page_table[pt_index].present);

		return (uintptr_t)page_table[pt_index].page() + get_offset(virt_addr);
	}

	paging_space_t create_kernel_space()
	{
		// Create and map the page directory to 0xFFFFF000
		auto *page_directory_ptr = (page_directory_t *)kcalloc(PAGE_SIZE, PAGE_SIZE);
		auto *mapping_table_ptr = (page_table_t *)kcalloc(PAGE_SIZE, PAGE_SIZE);

		assert(page_directory_ptr);
		assert(mapping_table_ptr);

		auto &page_directory = *page_directory_ptr;
		auto &mapping_table = *mapping_table_ptr;

		uintptr_t pt_address = (uintptr_t)&mapping_table - (uintptr_t)&_virtual_addr;
		uintptr_t pd_address = (uintptr_t)&page_directory - (uintptr_t)&_virtual_addr;

		page_directory[PAGE_COUNT - 1] = raw_create_pde(pt_address, false, true, false);
		mapping_table[PAGE_COUNT - 1] = raw_create_pte(pd_address, false, true, false);

		// Calculate the amount of pages needed for the kernel and map it
		uintptr_t p_address = (uintptr_t)&_physical_addr;
		size_t size = (uintptr_t)&_kernel_end - (uintptr_t)&_kernel_start;
		uintptr_t v_address = (uintptr_t)&_kernel_start;

		uintptr_t page_limit = LibK::round_up_to_multiple<uintptr_t>(p_address + size + 1, PAGE_SIZE);

		for (; p_address < page_limit; p_address += PAGE_SIZE, v_address += PAGE_SIZE)
		{
			size_t pd_index = get_pd_index(v_address);

			if (page_directory[pd_index].value() == 0)
			{
				// NOTE: At this early stage we rely on the preallocated heap
				// as this paging space is not yet installed and as such we
				// cannot use the mapping technique yet.
				auto *page_table_ptr = (page_table_t *)kcalloc(PAGE_SIZE, PAGE_SIZE);
				assert(page_table_ptr);

				auto &page_table = *page_table_ptr;

				uintptr_t address = (uintptr_t)&page_table - (uintptr_t)&_virtual_addr;

				page_directory[pd_index] = raw_create_pde(address, false, true, false);
				mapping_table[pd_index] = raw_create_pte(address, false, true, false);
			}

			auto &page_table = *(page_table_t *)((uintptr_t)page_directory[pd_index].table() + (uintptr_t)&_virtual_addr);

			size_t pt_index = get_pt_index(v_address);
			uintptr_t page_address = to_page_address(p_address);

			page_table[pt_index] = raw_create_pte(page_address, false, true, false);
		}

		s_kernel_paging_space = paging_space_t{
			.physical_pd_address = pd_address,
			.page_directory = &page_directory,
			.mapping_table = &mapping_table,
		};

		return s_kernel_paging_space;
	}

	// Create a new empty memory space where only kernel space is mapped and userspace is empty
	paging_space_t create_memory_space(paging_space_t &kernel_space)
	{
		auto *page_directory_ptr = (page_directory_t *)kcalloc(PAGE_SIZE, PAGE_SIZE);
		auto *mapping_table_ptr = (page_table_t *)kcalloc(PAGE_SIZE, PAGE_SIZE);

		assert(page_directory_ptr);
		assert(mapping_table_ptr);

		auto &page_directory = *page_directory_ptr;
		auto &mapping_table = *mapping_table_ptr;

		size_t kernel_start_pd_index = get_pd_index((uintptr_t)&_virtual_addr);
		size_t byte_size = (PAGE_COUNT - kernel_start_pd_index) * sizeof(page_directory_entry_t);

		void *pd_src = &kernel_space.page_directory->tables[kernel_start_pd_index];
		void *pd_dest = &page_directory[kernel_start_pd_index];
		memcpy(pd_dest, pd_src, byte_size);

		void *mp_src = &kernel_space.mapping_table->pages[kernel_start_pd_index];
		void *mp_dest = &mapping_table[kernel_start_pd_index];
		memcpy(mp_dest, mp_src, byte_size);

		page_directory[PAGE_COUNT - 1].page_table_address = as_physical((uintptr_t)mapping_table_ptr) >> 12;
		mapping_table[PAGE_COUNT - 1].page_address = as_physical((uintptr_t)page_directory_ptr) >> 12;

		return paging_space_t{
		    .physical_pd_address = as_physical((uintptr_t)page_directory_ptr),
		    .page_directory = &page_directory,
		    .mapping_table = &mapping_table,
		};
	}

	void map(paging_space_t &memory_space, uintptr_t phys_addr, uintptr_t virt_addr, size_t size, mapping_config_t config)
	{
		assert(virt_addr + size < PAGE_TABLE_ARRAY_ADDR);

		for_page_in_range(virt_addr, size, [&](uintptr_t virt_addr) {
			size_t pd_index = get_pd_index(virt_addr);
			size_t pt_index = get_pt_index(virt_addr);

			auto &page_directory = get_page_directory();

			if (page_directory[pd_index].value() == 0)
				page_directory[pd_index] = create_pde(pd_index, memory_space, config.userspace, config.writeable, !config.cacheable);

			assert(page_directory[pd_index].present);

			auto &page_table = get_page_table(pd_index);

			assert(!page_table[pt_index].present);

			uintptr_t page_address = to_page_address(phys_addr);
			phys_addr += PAGE_SIZE;

			page_table[pt_index] = create_pte(page_address, page_directory[pd_index], config.userspace, config.writeable, !config.cacheable);

			invalidate(virt_addr);
		});
	}

	void unmap(__unused paging_space_t &memory_space, uintptr_t virt_addr, size_t size)
	{
		assert(virt_addr + size < PAGE_TABLE_ARRAY_ADDR);

		LibK::vector<size_t> page_tables_to_check = LibK::vector<size_t>((size + TABLE_SIZE - 1) / TABLE_SIZE);
		size_t current = PAGE_COUNT;
		size_t idx = 0;

		for_page_in_range(virt_addr, size, [&](uintptr_t virt_addr) {
			static const page_table_entry_t null_pt_entry{};

			size_t pd_index = get_pd_index(virt_addr);
			auto &page_directory = get_page_directory();
			assert(page_directory[pd_index].present);

			size_t pt_index = get_pt_index(virt_addr);
			auto &page_table = get_page_table(pd_index);
			assert(page_table[pt_index].present);

			page_table[pt_index] = null_pt_entry;

			if (current != pd_index)
			{
				page_tables_to_check[idx] = pd_index;
				current = pd_index;
			}

			invalidate(virt_addr);
		});

		static const page_directory_entry_t null_pd_entry{};

		// Check if page directories have become empty
		for (auto pd_index : page_tables_to_check)
		{
			auto &page_table = get_page_table(pd_index);
			bool to_delete = true;

			for (size_t i = 0; i < PAGE_COUNT; i++)
			{
				if (page_table[i].value() != 0)
				{
					to_delete = false;
					break;
				}
			}

			if (to_delete)
			{
				auto &page_directory = get_page_directory();
				void *phys_addr = (void *)page_directory[pd_index].table();
				page_directory[pd_index] = null_pd_entry;
				PhysicalMemoryManager::instance().free(phys_addr, PAGE_SIZE);
			}
		}
	}

	memory_region_t get_kernel_region()
	{
		size_t size = (uintptr_t)&_kernel_end - (uintptr_t)&_kernel_start;
		uintptr_t v_address = (uintptr_t)&_kernel_start;
		uintptr_t p_address = (uintptr_t)&_physical_addr;

		uintptr_t page_begin = v_address / PAGE_SIZE * PAGE_SIZE;
		uintptr_t page_end = LibK::round_up_to_multiple<uintptr_t>(v_address + size + 1, PAGE_SIZE);

		uintptr_t phys_begin = LibK::round_down_to_multiple<uintptr_t>(p_address, PAGE_SIZE);

		return memory_region_t{
		    .virt_address = page_begin,
		    .phys_address = phys_begin,
		    .size = page_end - page_begin,
		    .mapped = true,
		    .present = true,
		    .allocated = true,
		    .config = {},
		};
	}

	memory_region_t get_mapping_region()
	{
		return memory_region_t{
		    .virt_address = PAGE_TABLE_ARRAY_ADDR,
		    .phys_address = 0, // does not apply here
		    .size = TABLE_SIZE,
		    .mapped = true,
		    .present = true,
		    .allocated = true,
		    .config = {},
		};
	}

	void load(paging_space_t &memory_space)
	{
		assert(memory_space.page_directory);
		assert(memory_space.mapping_table);

		CPU::Processor::load_page_directory(memory_space.physical_pd_address);
	}

	void invalidate(uintptr_t virtual_address)
	{
		CPU::Processor::invalidate_address(virtual_address);
	}

	Arch::paging_space_t get_kernel_paging_space()
	{
		return s_kernel_paging_space;
	}
} // namespace Kernel::Memory::Arch