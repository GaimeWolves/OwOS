#pragma once

#include <stddef.h>
#include <stdint.h>

#include <arch/memory.hpp>
#include <arch/spinlock.hpp>
#include <memory/definitions.hpp>
#include <multiboot.h>

#include <libk/kfunctional.hpp>
#include <libk/kvector.hpp>
#include <libk/AVLTree.hpp>

namespace Kernel::Memory
{
	typedef struct memory_space_t
	{
		Arch::paging_space_t paging_space;
		LibK::AVLTree<memory_region_t> userland_map;
	} memory_space_t;

	class VirtualMemoryManager
	{
	public:
		static VirtualMemoryManager &instance()
		{
			static VirtualMemoryManager *instance{nullptr};

			if (!instance)
				instance = new VirtualMemoryManager();

			return *instance;
		}

		VirtualMemoryManager(VirtualMemoryManager &) = delete;
		void operator=(const VirtualMemoryManager &) = delete;

		void init(multiboot_info_t *&mulitboot_info);

		memory_region_t allocate_region(size_t size, mapping_config_t config = {});
		memory_region_t allocate_region_at(uintptr_t virt_addr, size_t size, mapping_config_t config = {});

		// NOTE: These methods won't mark the physical regions as used, as such other users may change the memory
		memory_region_t map_region(uintptr_t phys_addr, size_t size, mapping_config_t config = {});
		memory_region_t map_region_at(uintptr_t phys_addr, uintptr_t virt_addr, size_t size, mapping_config_t config = {});

		memory_region_t map_region_identity(uintptr_t phys_addr, size_t size, mapping_config_t config = {})
		{
			return map_region_at(phys_addr, phys_addr, size, config);
		}

		template <typename T>
		T *map_typed(uintptr_t phys_addr, mapping_config_t config = {})
		{
			size_t offset = phys_addr - LibK::round_down_to_multiple<size_t>(phys_addr, PAGE_SIZE);
			auto region = map_region(phys_addr, sizeof(T) + offset, config);
			return reinterpret_cast<T *>(region.virt_address + offset);
		}

		void free(void *ptr);
		void free(const memory_region_t &region);

		[[nodiscard]] const memory_region_t *find_region(uintptr_t virtual_addr);

		void enumerate(const LibK::function<bool(memory_region_t)> &callback);

		static void load_memory_space(memory_space_t *memory_space);
		[[nodiscard]] static memory_space_t create_memory_space();
		[[nodiscard]] memory_space_t *get_kernel_memory_space() { return &m_kernel_memory_space; }

	private:
		VirtualMemoryManager() = default;
		~VirtualMemoryManager() = default;

		[[nodiscard]] bool check_free(const region_t &region) const;

		[[nodiscard]] region_t find_free_region(size_t size, bool is_kernel_space) const;

		memory_region_t map(uintptr_t phys_address, uintptr_t virt_address, size_t size, mapping_config_t config);
		void unmap(const memory_region_t &region);

		void traverse_all(bool is_kernel_space, const LibK::function<bool(memory_region_t)> &callback) const;
		void traverse_unmapped(bool is_kernel_space, const LibK::function<bool(memory_region_t)> &callback) const;
		void traverse_mapped(bool is_kernel_space, const LibK::function<bool(memory_region_t)> &callback) const;

		bool in_kernel_space(uintptr_t virt_address) const;

		memory_space_t m_kernel_memory_space{};
		Arch::paging_space_t m_kernel_paging_space{};
		LibK::AVLTree<memory_region_t> m_kernel_memory_map{};

		// TODO: Implement a lockless design
		Locking::Spinlock m_lock{};
	};
} // namespace Kernel::Memory