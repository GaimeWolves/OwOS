#include <arch/memory.hpp>

#include <arch/Processor.hpp>
#include <arch/i686/msr.hpp>
#include <arch/spinlock.hpp>
#include <common_attributes.h>
#include <logging/logger.hpp>
#include <memory/PhysicalMemoryManager.hpp>

#include <libk/kcassert.hpp>
#include <libk/kcmalloc.hpp>
#include <libk/kcstring.hpp>
#include <libk/kfunctional.hpp>
#include <libk/kmath.hpp>
#include <libk/kmemory.hpp>
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

#define PAT_MODE_UNCACHEABLE     0
#define PAT_MODE_WRITE_COMBINING 1
#define PAT_MODE_WRITETHROUGH    4
#define PAT_MODE_WRITE_PROTECT   5
#define PAT_MODE_WRITEBACK       6
#define PAT_MODE_UNCACHED        7

#define CACHE_MODE_INDEX_UNCACHEABLE     3
#define CACHE_MODE_INDEX_WRITE_COMBINING 4
#define CACHE_MODE_INDEX_WRITE_PROTECT   5
#define CACHE_MODE_INDEX_WRITETHROUGH    1
#define CACHE_MODE_INDEX_WRITEBACK       0

extern "C"
{
	extern uintptr_t _virtual_addr;
	extern uintptr_t _physical_addr;
	extern uintptr_t _kernel_start;
	extern uintptr_t _kernel_end;
}

// TODO: Refactor this to use the same page tables for kernel addresses

namespace Kernel::Memory::Arch
{
	class PageInvalidatedMessage final : public CPU::ProcessorMessage
	{
	public:
		explicit PageInvalidatedMessage(uintptr_t address)
		    : m_address(address)
		{
		}

		void handle() override
		{
			CPU::Processor::invalidate_address(m_address);
			// log("SMP", "Invalidated page %p", m_address);
		}

	private:
		uintptr_t m_address{};
	};

	class DirectoryInvalidatedMessage final : public CPU::ProcessorMessage
	{
	public:
		void handle() override
		{
			check_page_directory();
			log("SMP", "Synchronized page directory");
		}
	};

	inline static constexpr size_t to_page_address(uintptr_t phys_addr);
	inline static constexpr size_t to_directory_address(uintptr_t phys_addr);

	inline static constexpr size_t get_pd_index(uintptr_t virt_addr);
	inline static constexpr size_t get_pt_index(uintptr_t virt_addr);
	inline static constexpr size_t get_offset(uintptr_t virt_addr);

	inline static page_directory_t &get_page_directory();
	inline static page_table_t &get_page_table(size_t pd_index);

	inline static bool get_write_through_from_pat_index(uint8_t pat_index);
	inline static bool get_cache_disable_from_pat_index(uint8_t pat_index);
	inline static bool get_page_attribute_from_pat_index(uint8_t pat_index);

	inline static page_directory_entry_t raw_create_pde(uintptr_t table_addr, bool is_user, bool is_writeable, uint8_t caching_mode);
	inline static page_table_entry_t raw_create_pte(uintptr_t page_addr, bool is_user, bool is_writeable, uint8_t caching_mode);

	static page_directory_entry_t create_pde(size_t pd_index, paging_space_t &memory_space, bool is_user, bool is_writeable, uint8_t caching_mode);
	static page_table_entry_t create_pte(uintptr_t page_address, page_directory_entry_t &pde, bool is_user, bool is_writeable, uint8_t caching_mode);

	static void for_page_in_range(uintptr_t virt_addr, size_t size, LibK::function<void(uintptr_t)> callback);

	static uint8_t caching_mode_to_pat_index(CachingMode mode);

	static paging_space_t s_kernel_paging_space{};

	static Locking::Spinlock s_page_directory_lock{};
	static page_directory_t *s_master_page_directory;
	static page_table_t *s_master_mapping_table;
	static uint64_t s_page_directory_version;

	inline static bool is_kernel_space(uintptr_t virt_addr)
	{
		return virt_addr > reinterpret_cast<uintptr_t>(&_virtual_addr);
	}

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

	inline static bool get_write_through_from_pat_index(uint8_t pat_index)
	{
		return pat_index & 1;
	}

	inline static bool get_cache_disable_from_pat_index(uint8_t pat_index)
	{
		return (pat_index >> 1) & 1;
	}

	inline static bool get_page_attribute_from_pat_index(uint8_t pat_index)
	{
		return (pat_index >> 2) & 1;
	}

	inline static page_directory_entry_t raw_create_pde(uintptr_t table_addr, bool is_user, bool is_writeable, uint8_t pat_index)
	{
		return page_directory_entry_t{
		    .present = true,
		    .writeable = is_writeable,
		    .user = is_user,
		    .write_through = get_write_through_from_pat_index(pat_index),
		    .cache_disable = get_cache_disable_from_pat_index(pat_index),
		    .accessed = false,
		    .page_size = false,
		    .page_table_address = (uint32_t)table_addr >> OFFSET_BITS,
		};
	}

	inline static page_table_entry_t raw_create_pte(uintptr_t page_addr, bool is_user, bool is_writeable, uint8_t pat_index)
	{
		return page_table_entry_t{
		    .present = true,
		    .writeable = is_writeable,
		    .user = is_user,
		    .write_through = get_write_through_from_pat_index(pat_index),
		    .cache_disable = get_cache_disable_from_pat_index(pat_index),
		    .accessed = false,
		    .dirty = false,
		    .page_attribute = get_page_attribute_from_pat_index(pat_index),
		    .global = false,
		    .page_address = (uint32_t)page_addr >> OFFSET_BITS,
		};
	}

	static page_directory_entry_t create_pde(size_t pd_index, paging_space_t &memory_space, bool is_user, bool is_writeable, uint8_t pat_index)
	{
		uintptr_t table_addr = (uintptr_t)PhysicalMemoryManager::instance().alloc(PAGE_SIZE);

		auto &mapping_table = *memory_space.mapping_table;
		mapping_table[pd_index] = raw_create_pte(table_addr, false, true, caching_mode_to_pat_index(CachingMode::Uncacheable));

		// Clear it immediately to prevent bugs when we use this page table
		auto page_table = &get_page_table(pd_index);
		invalidate((uintptr_t)page_table);
		memset(page_table, 0, PAGE_SIZE);

		return raw_create_pde(table_addr, is_user, is_writeable, pat_index);
	}

	static page_table_entry_t create_pte(uintptr_t page_address, page_directory_entry_t &pde, bool is_user, bool is_writeable, uint8_t pat_index)
	{
		// User pages are only allowed in user page directories
		assert(!is_user || (is_user && pde.user));

		return raw_create_pte(page_address, is_user, is_writeable, pat_index);
	}

	static void for_page_in_range(uintptr_t virt_addr, size_t size, LibK::function<void(uintptr_t)> callback)
	{
		uintptr_t page_limit = LibK::round_up_to_multiple<uintptr_t>(virt_addr + size, PAGE_SIZE);

		for (; virt_addr < page_limit; virt_addr += PAGE_SIZE)
			callback(virt_addr);
	}

	static uint8_t caching_mode_to_pat_index(CachingMode mode)
	{
		switch (mode)
		{
		case CachingMode::Uncacheable:
			return CACHE_MODE_INDEX_UNCACHEABLE;
		case CachingMode::WriteCombining:
			return CACHE_MODE_INDEX_WRITE_COMBINING;
		case CachingMode::WriteThrough:
			return CACHE_MODE_INDEX_WRITETHROUGH;
		case CachingMode::WriteProtect:
			return CACHE_MODE_INDEX_WRITE_PROTECT;
		case CachingMode::WriteBack:
			return CACHE_MODE_INDEX_WRITEBACK;
		}

		return CACHE_MODE_INDEX_WRITETHROUGH;
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

	void initialize()
	{
		uint64_t pat = 0;
		pat |= (uint64_t)PAT_MODE_WRITEBACK;
		pat |= (uint64_t)PAT_MODE_WRITETHROUGH << 8;
		pat |= (uint64_t)PAT_MODE_UNCACHED << 16;
		pat |= (uint64_t)PAT_MODE_UNCACHEABLE << 24;
		pat |= (uint64_t)PAT_MODE_WRITE_COMBINING << 32;
		pat |= (uint64_t)PAT_MODE_WRITE_PROTECT << 40;
		pat |= (uint64_t)PAT_MODE_UNCACHED << 48;
		pat |= (uint64_t)PAT_MODE_UNCACHEABLE << 56;

		write_msr(IA32_PAT_MSR, pat);
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

		page_directory[PAGE_COUNT - 1] = raw_create_pde(pt_address, false, true, caching_mode_to_pat_index(CachingMode::Uncacheable));
		mapping_table[PAGE_COUNT - 1] = raw_create_pte(pd_address, false, true, caching_mode_to_pat_index(CachingMode::Uncacheable));

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

				page_directory[pd_index] = raw_create_pde(address, false, true, caching_mode_to_pat_index(CachingMode::Uncacheable));
				mapping_table[pd_index] = raw_create_pte(address, false, true, caching_mode_to_pat_index(CachingMode::Uncacheable));
			}

			auto &page_table = *(page_table_t *)((uintptr_t)page_directory[pd_index].table() + (uintptr_t)&_virtual_addr);

			size_t pt_index = get_pt_index(v_address);
			uintptr_t page_address = to_page_address(p_address);

			page_table[pt_index] = raw_create_pte(page_address, false, true, caching_mode_to_pat_index(CachingMode::Uncacheable));
		}

		s_kernel_paging_space = paging_space_t{
		    .physical_pd_address = pd_address,
		    .page_directory = &page_directory,
		    .mapping_table = &mapping_table,
		    .page_directory_version = 0,
		};

		s_master_page_directory = page_directory_ptr;
		s_master_mapping_table = mapping_table_ptr;
		s_page_directory_version = 0;

		return s_kernel_paging_space;
	}

	// Create a new empty memory space where only kernel space is mapped and userspace is empty
	paging_space_t create_memory_space()
	{
		auto *page_directory_ptr = (page_directory_t *)kcalloc(PAGE_SIZE, PAGE_SIZE);
		auto *mapping_table_ptr = (page_table_t *)kcalloc(PAGE_SIZE, PAGE_SIZE);

		assert(page_directory_ptr);
		assert(mapping_table_ptr);

		auto &page_directory = *page_directory_ptr;
		auto &mapping_table = *mapping_table_ptr;

		size_t kernel_start_pd_index = get_pd_index((uintptr_t)&_virtual_addr);
		size_t byte_size = (PAGE_COUNT - kernel_start_pd_index) * sizeof(page_directory_entry_t);

		s_page_directory_lock.lock();
		void *pd_src = &s_master_page_directory->tables[kernel_start_pd_index];
		void *pd_dest = &page_directory[kernel_start_pd_index];
		memcpy(pd_dest, pd_src, byte_size);

		void *mp_src = &s_master_mapping_table->pages[kernel_start_pd_index];
		void *mp_dest = &mapping_table[kernel_start_pd_index];
		memcpy(mp_dest, mp_src, byte_size);

		uint64_t version = s_page_directory_version;
		s_page_directory_lock.unlock();

		page_directory[PAGE_COUNT - 1].page_table_address = as_physical((uintptr_t)mapping_table_ptr) >> 12;
		mapping_table[PAGE_COUNT - 1].page_address = as_physical((uintptr_t)page_directory_ptr) >> 12;

		return paging_space_t{
		    .physical_pd_address = as_physical((uintptr_t)page_directory_ptr),
		    .page_directory = &page_directory,
		    .mapping_table = &mapping_table,
		    .page_directory_version = version,
		};
	}

	void map(paging_space_t &memory_space, uintptr_t phys_addr, uintptr_t virt_addr, size_t size, mapping_config_t config)
	{
		assert(virt_addr + size < PAGE_TABLE_ARRAY_ADDR);

		uint8_t pat_index = caching_mode_to_pat_index(config.caching_mode);

		for_page_in_range(virt_addr, size, [&](uintptr_t virt_addr) {
			size_t pd_index = get_pd_index(virt_addr);
			size_t pt_index = get_pt_index(virt_addr);

			auto &page_directory = get_page_directory();

			if (page_directory[pd_index].value() == 0)
			{
				if (is_kernel_space(virt_addr))
				{
					s_page_directory_lock.lock();
					if (s_master_page_directory->tables[pd_index].value() == 0)
					{
						s_master_page_directory->tables[pd_index] = create_pde(pd_index, memory_space, config.userspace, config.writeable, caching_mode_to_pat_index(CachingMode::Uncacheable));
						s_master_mapping_table->pages[pd_index] = raw_create_pte(s_master_page_directory->tables[pd_index].page_table_address << 12, false, true, caching_mode_to_pat_index(CachingMode::Uncacheable));
						s_page_directory_version++;
						auto &processor = CPU::Processor::current();
						processor.get_memory_space()->paging_space.page_directory_version = s_page_directory_version;
						processor.smp_broadcast(LibK::make_shared<DirectoryInvalidatedMessage>(), true);
					}
					else
					{
						page_directory[pd_index] = s_master_page_directory->tables[pd_index];
					}
					s_page_directory_lock.unlock();
				}
				else
				{
					page_directory[pd_index] = create_pde(pd_index, memory_space, config.userspace, config.writeable, pat_index);
				}
			}

			assert(page_directory[pd_index].present);

			auto &page_table = get_page_table(pd_index);

			assert(!page_table[pt_index].present);

			uintptr_t page_address = to_page_address(phys_addr);
			phys_addr += PAGE_SIZE;

			page_table[pt_index] = create_pte(page_address, page_directory[pd_index], config.userspace, config.writeable, pat_index);

			invalidate(virt_addr);
		});
	}

	void unmap(__unused paging_space_t &memory_space, uintptr_t virt_addr, size_t size)
	{
		assert(virt_addr + size < PAGE_TABLE_ARRAY_ADDR);

		LibK::vector<size_t> page_tables_to_check = LibK::vector<size_t>();
		size_t current = PAGE_COUNT;

		for_page_in_range(virt_addr, size, [&](uintptr_t virt_addr) {
			static const page_table_entry_t null_pt_entry{};

			size_t pd_index = get_pd_index(virt_addr);
			auto &page_directory = get_page_directory();
			assert(page_directory[pd_index].present);

			size_t pt_index = get_pt_index(virt_addr);
			auto &page_table = get_page_table(pd_index);
			assert(page_table[pt_index].present);

			page_table[pt_index] = null_pt_entry;

			// TODO: Implement deletion of kernel space page directories
			if (current != pd_index && !is_kernel_space(virt_addr))
			{
				page_tables_to_check.push_back(pd_index);
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
		if (is_kernel_space(virtual_address))
			CPU::Processor::current().smp_broadcast(LibK::make_shared<PageInvalidatedMessage>(virtual_address), true);
	}

	Arch::paging_space_t get_kernel_paging_space()
	{
		return s_kernel_paging_space;
	}

	// TODO: Upgrade to using journaling
	void check_page_directory()
	{
		// NOTE: This is a critical section as getting preempted inside this operation can cause the data of the page directory loaded
		//       to not correspond to the version code loaded later.
		s_page_directory_lock.lock();
		paging_space_t &paging_space = CPU::Processor::current().get_memory_space()->paging_space;

		if (paging_space.page_directory_version != s_page_directory_version)
		{
			size_t pd_index = get_pd_index(reinterpret_cast<uintptr_t>(&_virtual_addr));
			size_t count = (PAGE_COUNT - pd_index - 1) * sizeof(page_directory_entry_t);
			memcpy(&get_page_directory().tables[pd_index], &s_master_page_directory->tables[pd_index], count);
			memcpy(&paging_space.mapping_table->pages[pd_index], &s_master_mapping_table->pages[pd_index], count);
			CPU::Processor::load_page_directory(paging_space.physical_pd_address);
			log("MEMORY", "Updated kernel page directory");
		}

		paging_space.page_directory_version = s_page_directory_version;
		s_page_directory_lock.unlock();
	}
} // namespace Kernel::Memory::Arch