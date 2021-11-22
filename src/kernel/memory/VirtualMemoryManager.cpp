#include <memory/VirtualMemoryManager.hpp>

#include <common_attributes.h>
#include <memory/PhysicalMemoryManager.hpp>
#include <panic.hpp>

#include <libk/kcmalloc.hpp>
#include <libk/kcstdio.hpp>
#include <libk/kcstring.hpp>
#include <libk/kmath.hpp>

extern "C"
{
	extern uintptr_t _virtual_addr;
}

namespace Kernel::Memory
{
	static inline bool in_kernel_space(uintptr_t virt_addr);

	static inline bool in_kernel_space(uintptr_t virt_addr)
	{
		return virt_addr >= (uintptr_t)&_virtual_addr;
	}

	void VirtualMemoryManager::init(multiboot_info_t *&mulitboot_info)
	{
		PhysicalMemoryManager::instance().init(mulitboot_info);

		m_kernel_memory_space = (memory_space_t *)kcalloc(sizeof(memory_space_t));
		m_current_memory_space = m_kernel_memory_space;
		assert(m_kernel_memory_space);

		m_kernel_memory_space->paging_space = Arch::create_kernel_space();
		Arch::load(m_kernel_memory_space->paging_space);

		auto kernel_region = Arch::get_kernel_region();
		auto mapping_region = Arch::get_mapping_region();

		m_kernel_memory_space->kernel_map = avl_insert(m_kernel_memory_space->kernel_map, kernel_region);
		m_kernel_memory_space->kernel_map = avl_insert(m_kernel_memory_space->kernel_map, mapping_region);
	}

	void *VirtualMemoryManager::alloc_kernel_buffer(size_t size, RegionType type, AllocationStategy strategy)
	{
		size = LibK::round_up_to_multiple<size_t>(size, PAGE_SIZE);

		region_t region = find_free_region(size, m_current_memory_space->kernel_map, strategy);

		uintptr_t phys_addr = 0;

		switch (type)
		{
		case RegionType::ISA_DMA: // Respects 64 KiB boundary
			assert(size <= 65536);
			phys_addr = (uintptr_t)PhysicalMemoryManager::instance().alloc(size, 0, UINTPTR_MAX, 65536);
			break;
		case RegionType::Normal:
			phys_addr = (uintptr_t)PhysicalMemoryManager::instance().alloc(size);
		}

		auto mem_region = map(phys_addr, region.address, size, false);

		return (void *)mem_region.region.address;
	}

	// Directly maps the provided physical region to a virtual region
	void *VirtualMemoryManager::alloc_mmio_buffer(uintptr_t phys_addr, size_t size, __unused RegionType type, AllocationStategy strategy)
	{
		size = LibK::round_up_to_multiple<size_t>(size, PAGE_SIZE);

		region_t region = find_free_region(size, m_current_memory_space->kernel_map, strategy);
		auto mem_region = map(phys_addr, region.address, size, false);
		mem_region.is_mmio = true;

		return (void *)mem_region.region.address;
	}

	void VirtualMemoryManager::free(void *ptr)
	{
		auto region = find_region((uintptr_t)ptr);

		assert(region.mapped);

		unmap(region.region.address);

		if (region.is_mmio)
			return;

		assert(region.present); // Swapping not yet implemented

		PhysicalMemoryManager::instance().free((void *)region.phys_address, region.region.size);
	}

	region_t VirtualMemoryManager::find_free_region(size_t size, vm_avl_node_t *memory_space, AllocationStategy strategy)
	{
		assert(size > 0);

		size_t min_size = SIZE_MAX;
		region_t min_region{0, 0};

		auto callback = LibK::function<bool(region_t)>([&](region_t region) {
			if (strategy == AllocationStategy::FirstFit)
			{
				if (region.size >= size)
				{
					min_region = region;
					min_size = region.size;
					return false; // Break traversal
				}
			}
			else
			{
				if (region.size < min_size && region.size >= size)
				{
					min_region = region;
					min_size = region.size;

					if (min_size == size) // Perfect fit
						return false;
				}

				return true;
			}

			return true;
		});

		avl_traverse_unmapped(true, memory_space, callback);

		// TODO: Implement more metadata to determine memory (like cache) that can be unmapped
		if (min_region.size == 0)
			panic("Out of kernel virtual memory (OOM) while allocating buffer of size %u", size);

		return min_region;
	}

	memory_region_t VirtualMemoryManager::find_region(uintptr_t virtual_addr)
	{
		auto *tree = virtual_addr > (uintptr_t)&_virtual_addr ? m_current_memory_space->kernel_map : m_current_memory_space->userland_map;

		auto *node = avl_find(tree, virtual_addr);

		if (node)
			return node->region;

		return memory_region_t();
	}

	void VirtualMemoryManager::enumerate_mapped(LibK::function<bool(memory_region_t)> callback)
	{
		auto callback_trampoline = LibK::function<bool(vm_avl_node_t *)>([&](vm_avl_node_t *node) {
			return callback(node->region);
		});

		avl_traverse_mapped(false, m_current_memory_space->userland_map, callback_trampoline);
		avl_traverse_mapped(true, m_current_memory_space->kernel_map, callback_trampoline);
	}

	void VirtualMemoryManager::enumerate_unmapped(LibK::function<bool(memory_region_t)> callback)
	{
		auto callback_trampoline = LibK::function<bool(region_t)>([&](region_t region) {
			return callback(memory_region_t{
			    .region = region,
			    .phys_address = 0,
			    .mapped = false,
			    .present = false,
			    .kernel = in_kernel_space(region.address),
			    .is_mmio = false,
			});
		});

		avl_traverse_unmapped(false, m_current_memory_space->userland_map, callback_trampoline);
		avl_traverse_unmapped(true, m_current_memory_space->kernel_map, callback_trampoline);
	}

	void VirtualMemoryManager::enumerate_all(LibK::function<bool(memory_region_t)> callback)
	{
		auto callback_trampoline = LibK::function<bool(region_t, vm_avl_node_t *)>([&](region_t region, vm_avl_node_t *node) {
			if (node)
				return callback(node->region);
			else
			{
				return callback(memory_region_t{
				    .region = region,
				    .phys_address = 0,
				    .mapped = false,
				    .present = false,
				    .kernel = in_kernel_space(region.address),
				    .is_mmio = false,
				});
			}
		});

		avl_traverse_all(false, m_current_memory_space->userland_map, callback_trampoline);
		avl_traverse_all(true, m_current_memory_space->kernel_map, callback_trampoline);
	}

	void VirtualMemoryManager::load_kernel_space()
	{
		m_current_memory_space = m_kernel_memory_space;
		load_memory_space(m_kernel_memory_space);
	}

	void VirtualMemoryManager::load_memory_space(memory_space_t *memory_space)
	{
		assert(memory_space);

		m_current_memory_space = memory_space;
		Arch::load(memory_space->paging_space);
	}

	memory_region_t VirtualMemoryManager::map(uintptr_t physical_addr, uintptr_t virtual_addr, size_t size, bool is_user)
	{
		auto is_kernel_space = in_kernel_space(virtual_addr);
		auto *&tree = is_kernel_space ? m_current_memory_space->kernel_map : m_current_memory_space->userland_map;
		auto region = memory_region_t{
		    .region = {virtual_addr, size},
		    .phys_address = physical_addr,
		    .mapped = true,
		    .present = true,
		    .kernel = !is_user,
		    .is_mmio = false,
		};

		tree = avl_insert(tree, region);

		Arch::map(m_current_memory_space->paging_space, physical_addr, virtual_addr, size, is_user);

		return region;
	}

	void VirtualMemoryManager::unmap(uintptr_t virtual_addr)
	{
		auto is_kernel_space = in_kernel_space(virtual_addr);
		auto *&tree = is_kernel_space ? m_current_memory_space->kernel_map : m_current_memory_space->userland_map;

		auto *node = avl_find(tree, virtual_addr);

		if (node)
		{
			Arch::unmap(m_current_memory_space->paging_space, node->region.region.address, node->region.region.size);
			tree = avl_delete(tree, node->region);
		}
	}

	vm_avl_node_t *VirtualMemoryManager::avl_find(vm_avl_node_t *root, uintptr_t virtual_addr)
	{
		vm_avl_node_t *current = root;

		while (current)
		{
			if (current->region.region.contains(virtual_addr))
				return current;
			else if (virtual_addr < current->region.region.address)
				current = current->left;
			else // virtual_addr > current->region.region.address
				current = current->right;
		}

		return nullptr;
	}

	vm_avl_node_t *VirtualMemoryManager::avl_find(vm_avl_node_t *root, memory_region_t region)
	{
		vm_avl_node_t *current = root;

		while (current)
		{
			if (current->region.region == region.region)
				return current;
			else if (current->region.region > region.region)
				current = current->left;
			else // current->region.region < region.region
				current = current->right;
		}

		return nullptr;
	}

	void VirtualMemoryManager::avl_traverse_all(bool is_kernel_space, vm_avl_node_t *root, LibK::function<bool(region_t, vm_avl_node_t *)> &callback)
	{
		bool do_continue = true;

		if (!root)
		{
			if (is_kernel_space)
				do_continue = callback({(uintptr_t)&_virtual_addr, UINTPTR_MAX - (uintptr_t)&_virtual_addr}, nullptr);
			else
				do_continue = callback({0, (uintptr_t)&_virtual_addr}, nullptr);

			return;
		}

		vm_avl_node_t *last = nullptr;
		avl_traverse_all_helper(is_kernel_space, root, &last, &do_continue, callback);

		if (!do_continue)
			return;

		// Last now points to the last node in the tree
		if (last)
		{
			uintptr_t end = last->region.region.end();

			if (is_kernel_space && end < UINTPTR_MAX)
				callback({end + 1, UINTPTR_MAX - end + 1}, nullptr);
			else if (end < (uintptr_t)&_virtual_addr)
				callback({end, (uintptr_t)&_virtual_addr - end}, nullptr);
		}
	}

	// This uses the 'last' pointer that points to the region immediatly preceding
	// the currently processed one to report unmapped regions to the callback.
	// If the callback at any point returns false the traversal is immediatly stopped.
	void VirtualMemoryManager::avl_traverse_all_helper(bool is_kernel_space, vm_avl_node_t *root, vm_avl_node_t **last, bool *do_continue, LibK::function<bool(region_t, vm_avl_node_t *)> &callback)
	{
		if (!root)
			return;

		if (!*do_continue)
			return;

		avl_traverse_all_helper(is_kernel_space, root->left, last, do_continue, callback);

		if (!*do_continue)
			return;

		if (*last)
		{
			uintptr_t start = (*last)->region.region.end() + 1;
			uintptr_t end = root->region.region.address;

			if (end - start > 0)
				*do_continue = callback({start, end - start}, nullptr);
		}
		else // root must be the leftmost node
		{
			uintptr_t end = root->region.region.address;

			if (is_kernel_space && end > (uintptr_t)&_virtual_addr)
				*do_continue = callback({(uintptr_t)&_virtual_addr, end - (uintptr_t)&_virtual_addr}, nullptr);
			else if (!is_kernel_space && end > 0)
				*do_continue = callback({0, end}, nullptr);
		}

		if (!*do_continue)
			return;

		*last = root;
		*do_continue = callback(root->region.region, root);

		if (!*do_continue)
			return;

		avl_traverse_all_helper(is_kernel_space, root->right, last, do_continue, callback);
	}

	void VirtualMemoryManager::avl_traverse_unmapped(bool is_kernel_space, vm_avl_node_t *root, LibK::function<bool(region_t)> &callback)
	{
		auto callback_trampoline = LibK::function<bool(region_t, vm_avl_node_t *)>([&](region_t region, vm_avl_node_t *node) {
			if (!node)
				return callback(region);

			return true;
		});

		avl_traverse_all(is_kernel_space, root, callback_trampoline);
	}

	void VirtualMemoryManager::avl_traverse_mapped(bool is_kernel_space, vm_avl_node_t *root, LibK::function<bool(vm_avl_node_t *)> &callback)
	{
		auto callback_trampoline = LibK::function<bool(region_t, vm_avl_node_t *)>([&](region_t, vm_avl_node_t *node) {
			if (node)
				return callback(node);

			return true;
		});

		avl_traverse_all(is_kernel_space, root, callback_trampoline);
	}

	vm_avl_node_t *VirtualMemoryManager::avl_insert(vm_avl_node_t *root, memory_region_t region)
	{
		if (!root)
		{
			auto *node = new vm_avl_node_t{
			    .parent = nullptr,
			    .left = nullptr,
			    .right = nullptr,
			    .region = region,
			    .height = 1,
			};

			return node;
		}

		assert(!root->region.region.overlaps(region.region));

		if (root->region.region == region.region)
		{
			root->region = region;
			return root;
		}

		if (region.region < root->region.region)
		{
			root->left = avl_insert(root->left, region);
			root->left->parent = root;
		}
		else if (region.region > root->region.region)
		{
			root->right = avl_insert(root->right, region);
			root->right->parent = root;
		}

		root->height = avl_calc_height(root);

		return avl_rebalance(root);
	}

	vm_avl_node_t *VirtualMemoryManager::avl_delete(vm_avl_node_t *root, memory_region_t region)
	{
		vm_avl_node_t *temp;

		if (!root)
			return nullptr;

		if (region.region < root->region.region)
			root->left = avl_delete(root->left, region);
		else if (region.region > root->region.region)
			root->right = avl_delete(root->right, region);
		else
		{
			if (root->left && root->right)
			{
				auto *successor = avl_find_min(root->right);
				root->region = successor->region;
				root->right = avl_delete(root->right, root->region);
			}
			else
			{
				temp = root;

				if (root->left)
					root = root->left;
				else if (root->right)
					root = root->right;
				else
					root = nullptr;

				delete temp;
			}
		}

		if (!root)
			return root;

		root->height = avl_calc_height(root);

		return avl_rebalance(root);
	}

	vm_avl_node_t *VirtualMemoryManager::avl_rebalance(vm_avl_node_t *root)
	{
		int bf = avl_get_balance(root);

		if (bf < -1)
		{
			if (avl_get_balance(root->left) == -1)
				return avl_rotate_right(root);
			else
				return avl_rotate_left_right(root);
		}

		if (bf > 1)
		{
			if (avl_get_balance(root->right) == 1)
				return avl_rotate_left(root);
			else
				return avl_rotate_right_left(root);
		}

		return root;
	}

	vm_avl_node_t *VirtualMemoryManager::avl_rotate_right(vm_avl_node_t *root)
	{
		assert(root);

		auto *child = root->left;
		auto *middle = child->right;

		root->left = middle;
		child->right = root;

		if (root->parent)
		{
			if (root->parent->left == root)
				root->parent->left = child;
			else
				root->parent->right = child;
		}

		child->parent = root->parent;
		root->parent = child;

		if (middle)
			middle->parent = root;

		root->height = avl_calc_height(root);
		child->height = avl_calc_height(child);

		return child;
	}

	vm_avl_node_t *VirtualMemoryManager::avl_rotate_left(vm_avl_node_t *root)
	{
		assert(root);

		auto *child = root->right;
		auto *middle = child->left;

		root->right = middle;
		child->left = root;

		if (root->parent)
		{
			if (root->parent->left == root)
				root->parent->left = child;
			else
				root->parent->right = child;
		}

		child->parent = root->parent;
		root->parent = child;

		if (middle)
			middle->parent = root;

		root->height = avl_calc_height(root);
		child->height = avl_calc_height(child);

		return child;
	}

	vm_avl_node_t *VirtualMemoryManager::avl_rotate_right_left(vm_avl_node_t *root)
	{
		assert(root);

		root->right = avl_rotate_right(root->right);
		return avl_rotate_left(root);
	}

	vm_avl_node_t *VirtualMemoryManager::avl_rotate_left_right(vm_avl_node_t *root)
	{
		assert(root);

		root->left = avl_rotate_left(root->left);
		return avl_rotate_right(root);
	}

	vm_avl_node_t *VirtualMemoryManager::avl_find_min(vm_avl_node_t *root)
	{
		auto *current = root;

		if (!current)
			return current;

		while (current->left)
			current = current->left;

		return current;
	}

	int VirtualMemoryManager::avl_get_balance(vm_avl_node_t *root)
	{
		return avl_get_height(root->right) - avl_get_height(root->left);
	}

	int VirtualMemoryManager::avl_get_height(vm_avl_node_t *node)
	{
		return node ? node->height : 0;
	}

	int VirtualMemoryManager::avl_calc_height(vm_avl_node_t *node)
	{
		return 1 + LibK::max(avl_get_height(node->left), avl_get_height(node->right));
	}

	vm_avl_node_t *VirtualMemoryManager::avl_replace_in_parent(vm_avl_node_t *old_node, vm_avl_node_t *new_node)
	{
		assert(old_node);

		if (old_node->parent)
		{
			if (old_node == old_node->parent->left)
				old_node->parent->left = new_node;
			else
				old_node->parent->right = new_node;
		}

		if (new_node)
			new_node->parent = old_node->parent;

		return new_node;
	}

#ifdef _DEBUG
	void VirtualMemoryManager::debug_avl_tree(vm_avl_node_t *root)
	{
		if (!root)
		{
			kprintf("EMPTY\n");
			return;
		}

		LibK::vector<vm_avl_node_t *> seen;
		LibK::vector<LibK::vector<vm_avl_node_t *>> slices;
		debug_avl_tree_helper(root, slices, seen, 0, 0);

		int depth = slices.size() - 1;
		int width = 1 << depth;
		int max_line_width = width * 10;

		for (size_t d = 0; d < slices.size(); d++)
		{
			size_t width = 1 << d;
			int line_width = width * 10;
			int padding = (max_line_width - line_width) / 2;

			if (padding > 0)
				kprintf("%*c", padding, ' ');

			for (size_t i = 0; i < width; i++)
			{
				vm_avl_node_t *node = slices[d].size() > i ? slices[d][i] : (vm_avl_node_t *)UINTPTR_MAX;

				if ((uintptr_t)node == UINTPTR_MAX)
					kprintf("          ");
				else if (node)
					kprintf(" %8X ", (uintptr_t)node);
				else
					kprintf("   NULL   ");
			}

			kputc('\n');
			kputc('\n');
		}
	}

	void VirtualMemoryManager::debug_avl_tree_helper(vm_avl_node_t *root, LibK::vector<LibK::vector<vm_avl_node_t *>> &slices, LibK::vector<vm_avl_node_t *> &seen, int depth, size_t index)
	{
		if (slices.size() == (size_t)depth)
			slices.push_back(LibK::vector<vm_avl_node_t *>());

		if (slices[depth].size() < index)
		{
			while (slices[depth].size() < index)
				slices[depth].push_back((vm_avl_node_t *)UINTPTR_MAX);
		}

		slices[depth].push_back(root);

		if (!root)
			return;

		for (auto other : seen)
		{
			if (other == root)
				return;
		}

		seen.push_back(root);

		debug_avl_tree_helper(root->left, slices, seen, depth + 1, index * 2);
		debug_avl_tree_helper(root->right, slices, seen, depth + 1, index * 2 + 1);
	}
#endif
} // namespace Kernel::Memory