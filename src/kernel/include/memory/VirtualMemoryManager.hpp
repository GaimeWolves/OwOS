#pragma once

#include <stddef.h>
#include <stdint.h>

#include <arch/memory.hpp>
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

		LibK::AVLTree<memory_region_t> kernel_map;
		LibK::AVLTree<memory_region_t> userland_map;
	} memory_space_t;

	enum class AllocationStategy
	{
		FirstFit,
		BestFit,
	};

	enum class RegionType
	{
		Normal,  // No contstraints
		ISA_DMA, // Low physical, 64K boundary
	};

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

		// TODO: Add posibility to mark allocated memory uncached or read-/writeonly
		// TODO: Extend API to make it easier to map specific physical addresses (which would need an use counter)
		// TODO: Add API to remap (resize) mapped regions
		void *alloc_kernel_buffer(size_t size, RegionType type = RegionType::Normal, AllocationStategy strategy = AllocationStategy::BestFit);
		void *alloc_mmio_buffer(uintptr_t phys_addr, size_t size, RegionType type = RegionType::Normal, AllocationStategy strategy = AllocationStategy::BestFit);
		void free(void *ptr);

		void *map_physical(uintptr_t phys_addr, size_t size, RegionType type = RegionType::Normal, AllocationStategy strategy = AllocationStategy::BestFit);

		template <typename T>
		T *map_typed(uintptr_t phys_addr, RegionType type = RegionType::Normal, AllocationStategy strategy = AllocationStategy::BestFit)
		{
			return (T *)map_physical(phys_addr, sizeof(T), type, strategy);
		}

		[[nodiscard]] const memory_region_t *find_region(uintptr_t virtual_addr) const;

		void enumerate(const LibK::function<bool(memory_region_t)> &callback) const;

		void load_kernel_space();
		void load_memory_space(memory_space_t *memory_space);

	private:
		VirtualMemoryManager() = default;
		~VirtualMemoryManager() = default;

		region_t find_free_region(size_t size, bool is_kernel_space, AllocationStategy strategy = AllocationStategy::BestFit) const;

		memory_region_t map(uintptr_t physical_addr, uintptr_t virtual_addr, size_t size, bool kernel);
		void unmap(uintptr_t virtual_addr);

		void traverse_all(bool is_kernel_space, const LibK::function<bool(memory_region_t)> &callback) const;
		void traverse_unmapped(bool is_kernel_space, const LibK::function<bool(memory_region_t)> &callback) const;
		void traverse_mapped(bool is_kernel_space, const LibK::function<bool(memory_region_t)> &callback) const;

		memory_space_t *m_current_memory_space;
		memory_space_t *m_kernel_memory_space;
	};
} // namespace Kernel::Memory