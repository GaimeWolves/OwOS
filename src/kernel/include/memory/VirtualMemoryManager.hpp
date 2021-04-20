#pragma once

#include <stddef.h>
#include <stdint.h>

#include <arch/memory.hpp>
#include <memory/definitions.hpp>
#include <multiboot.h>

#include <libk/kfunctional.hpp>
#include <libk/kvector.hpp>

namespace Kernel::Memory
{
	typedef struct vm_avl_node_t
	{
		vm_avl_node_t *parent;
		vm_avl_node_t *left;
		vm_avl_node_t *right;

		memory_region_t region;
		int height;
	} vm_avl_node_t;

	typedef struct memory_space_t
	{
		Arch::paging_space_t paging_space;

		vm_avl_node_t *kernel_map;
		vm_avl_node_t *userland_map;
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

		void *alloc_kernel_buffer(size_t size, RegionType type = RegionType::Normal, AllocationStategy strategy = AllocationStategy::BestFit);
		void *alloc_mmio_buffer(uintptr_t phys_addr, size_t size, RegionType type = RegionType::Normal, AllocationStategy strategy = AllocationStategy::BestFit);
		void free(void *ptr);

		memory_region_t find_region(uintptr_t virtual_addr);

		void enumerate_mapped(LibK::function<bool(memory_region_t)> callback);
		void enumerate_unmapped(LibK::function<bool(memory_region_t)> callback);
		void enumerate_all(LibK::function<bool(memory_region_t)> callback);

		void load_kernel_space();
		void load_memory_space(memory_space_t *memory_space);

	private:
		VirtualMemoryManager() = default;
		~VirtualMemoryManager() = default;

		region_t find_free_region(size_t size, vm_avl_node_t *memory_space, AllocationStategy strategy = AllocationStategy::BestFit);

		memory_region_t map(uintptr_t physical_addr, uintptr_t virtual_addr, size_t size, bool kernel);
		void unmap(uintptr_t virtual_addr);

		vm_avl_node_t *avl_find(vm_avl_node_t *root, uintptr_t virtual_addr);
		vm_avl_node_t *avl_find(vm_avl_node_t *root, memory_region_t region);

		void avl_traverse_all(bool is_kernel_space, vm_avl_node_t *root, LibK::function<bool(region_t, vm_avl_node_t *)> &callback);
		void avl_traverse_all_helper(bool is_kernel_space, vm_avl_node_t *root, vm_avl_node_t **last, bool *do_break, LibK::function<bool(region_t, vm_avl_node_t *)> &callback);

		void avl_traverse_unmapped(bool is_kernel_space, vm_avl_node_t *root, LibK::function<bool(region_t)> &callback);
		void avl_traverse_mapped(bool is_kernel_space, vm_avl_node_t *root, LibK::function<bool(vm_avl_node_t *)> &callback);

		vm_avl_node_t *avl_insert(vm_avl_node_t *root, memory_region_t region);
		vm_avl_node_t *avl_delete(vm_avl_node_t *root, memory_region_t region);

		vm_avl_node_t *avl_rotate_right(vm_avl_node_t *root);
		vm_avl_node_t *avl_rotate_left(vm_avl_node_t *root);
		vm_avl_node_t *avl_rotate_right_left(vm_avl_node_t *root);
		vm_avl_node_t *avl_rotate_left_right(vm_avl_node_t *root);

		int avl_get_balance(vm_avl_node_t *root);
		int avl_get_height(vm_avl_node_t *node);
		int avl_calc_height(vm_avl_node_t *node);

		vm_avl_node_t *avl_find_min(vm_avl_node_t *root);
		vm_avl_node_t *avl_replace_in_parent(vm_avl_node_t *old_node, vm_avl_node_t *new_node);

		memory_space_t *m_current_memory_space{nullptr};
		memory_space_t *m_kernel_memory_space{nullptr};

#ifdef _DEBUG
		void debug_avl_tree(vm_avl_node_t *root);
		void debug_avl_tree_helper(vm_avl_node_t *root, LibK::vector<LibK::vector<vm_avl_node_t *>> &slices, LibK::vector<vm_avl_node_t *> &seen, int depth, size_t index);
#endif
	};
} // namespace Kernel::Memory