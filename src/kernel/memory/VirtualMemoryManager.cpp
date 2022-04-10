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
	void VirtualMemoryManager::init(multiboot_info_t *&mulitboot_info)
	{
		PhysicalMemoryManager::instance().init(mulitboot_info);

		m_kernel_paging_space = Arch::create_kernel_space();
		Arch::load(m_kernel_paging_space);

		auto kernel_region = Arch::get_kernel_region();
		auto mapping_region = Arch::get_mapping_region();

		m_kernel_memory_map.insert(kernel_region);
		m_kernel_memory_map.insert(mapping_region);
	}

	memory_region_t VirtualMemoryManager::allocate_region(size_t size, mapping_config_t config)
	{
		size = LibK::round_up_to_multiple<size_t>(size, PAGE_SIZE);
		region_t region = find_free_region(size, !config.userspace);
		uintptr_t phys_addr = reinterpret_cast<uintptr_t>(PhysicalMemoryManager::instance().alloc(size, config.bounds.address, config.bounds.end(), config.alignment));
		return map(phys_addr, region.address, size, config);
	}

	memory_region_t VirtualMemoryManager::allocate_region_at(uintptr_t virt_addr, size_t size, mapping_config_t config)
	{
		uintptr_t address = LibK::round_up_to_multiple<uintptr_t>(virt_addr, PAGE_SIZE);
		size = LibK::round_up_to_multiple<size_t>(size + (virt_addr - address), PAGE_SIZE);
		virt_addr = address;

		region_t virt_region{
			.address = virt_addr,
			.size = size,
		};

		if (!check_free(virt_region))
			return {};

		uintptr_t phys_addr = reinterpret_cast<uintptr_t>(PhysicalMemoryManager::instance().alloc(size, config.bounds.address, config.bounds.end(), config.alignment));

		return map(phys_addr, virt_addr, size, config);
	}

	memory_region_t VirtualMemoryManager::map_region(uintptr_t phys_addr, size_t size, mapping_config_t config)
	{
		uintptr_t address = LibK::round_down_to_multiple<uintptr_t>(phys_addr, PAGE_SIZE);
		size = LibK::round_up_to_multiple<size_t>(size + (phys_addr - address), PAGE_SIZE);
		phys_addr = address;
		region_t region = find_free_region(size, !config.userspace);
		return map(phys_addr, region.address, size, config);
	}

	memory_region_t VirtualMemoryManager::map_region_at(uintptr_t phys_addr, uintptr_t virt_addr, size_t size, mapping_config_t config)
	{
		uintptr_t address = LibK::round_down_to_multiple<uintptr_t>(virt_addr, PAGE_SIZE);
		size = LibK::round_up_to_multiple<size_t>(size + (virt_addr - address), PAGE_SIZE);
		virt_addr = address;

		region_t virt_region{
		    .address = virt_addr,
		    .size = size,
		};

		if (!check_free(virt_region))
			return {};

		return map(phys_addr, virt_addr, size, config);
	}

	void VirtualMemoryManager::free(void *ptr)
	{
		auto region = find_region((uintptr_t)ptr);
		assert(region);
		free(*region);
	}

	void VirtualMemoryManager::free(const memory_region_t &region)
	{
		assert(region.mapped);

		unmap(region);

		if (!region.allocated)
			return;

		assert(region.present); // Swapping not yet implemented

		PhysicalMemoryManager::instance().free((void *)region.phys_address, region.size);
	}

	region_t VirtualMemoryManager::find_free_region(size_t size, bool is_kernel_space) const
	{
		assert(size > 0);

		size_t min_size = SIZE_MAX;
		region_t min_region{0, 0};

		traverse_unmapped(is_kernel_space, [&](memory_region_t mem_region) {
			auto region = mem_region.virt_region();

			if (region.size < min_size && region.size >= size)
			{
				min_region = region;
				min_size = region.size;

				if (min_size == size)
					return false;
			}

			return true;
		});

		// TODO: Implement more metadata to determine memory (like cache) that can be unmapped
		if (min_region.size == 0)
			panic("Out of kernel virtual memory (OOM) while allocating buffer of size %u", size);

		return min_region;
	}

	const memory_region_t *VirtualMemoryManager::find_region(uintptr_t virtual_addr) const
	{
		auto &tree = in_kernel_space(virtual_addr) ? m_kernel_memory_map : m_current_memory_space->userland_map;

		return tree.find([virtual_addr](memory_region_t region) {
			if (region.virt_region().contains(virtual_addr))
				return 0;
			else if (region.virt_address < virtual_addr)
				return -1;
			else
				return 1;
		});
	}

	void VirtualMemoryManager::enumerate(const LibK::function<bool(memory_region_t)> &callback) const
	{
		m_current_memory_space->userland_map.traverse(callback);
		m_kernel_memory_map.traverse(callback);
	}

	void VirtualMemoryManager::load_kernel_space()
	{
		m_current_memory_space = nullptr;
		Arch::load(m_kernel_paging_space);
	}

	memory_space_t VirtualMemoryManager::create_memory_space()
	{
		auto paging_space = Arch::create_memory_space(m_kernel_paging_space);

		return {
			.paging_space = paging_space,
			.userland_map = {}
		};
	}

	void VirtualMemoryManager::load_memory_space(memory_space_t *memory_space)
	{
		assert(memory_space);

		m_current_memory_space = memory_space;
		Arch::load(memory_space->paging_space);
	}

	memory_region_t VirtualMemoryManager::map(uintptr_t phys_address, uintptr_t virt_address, size_t size, mapping_config_t config)
	{
		auto &tree = in_kernel_space(virt_address) ? m_kernel_memory_map : m_current_memory_space->userland_map;

		auto region = memory_region_t{
		    .virt_address = virt_address,
		    .phys_address = phys_address,
		    .size = size,
		    .mapped = true,
		    .present = true,
		    .allocated = false,
		    .config = config,
		};

		tree.insert(region);

		if (m_current_memory_space)
			Arch::map(m_current_memory_space->paging_space, phys_address, virt_address, size, config);
		else
			Arch::map(m_kernel_paging_space, phys_address, virt_address, size, config);

		return region;
	}

	void VirtualMemoryManager::unmap(const memory_region_t &region)
	{
		auto &tree = in_kernel_space(region.virt_address) ? m_kernel_memory_map : m_current_memory_space->userland_map;

		if (m_current_memory_space)
			Arch::unmap(m_current_memory_space->paging_space, region.virt_address, region.size);
		else
			Arch::unmap(m_kernel_paging_space, region.virt_address, region.size);

		tree.remove(region);
	}

	bool VirtualMemoryManager::check_free(const region_t &region) const
	{
		bool free = true;

		traverse_mapped(in_kernel_space(region.address), [&](memory_region_t mem_region) {
			if (mem_region.virt_region().overlaps(region))
			{
				free = false;
				return false;
			}

			return true;
		});

		return free;
	}

	void VirtualMemoryManager::traverse_all(bool is_kernel_space, const LibK::function<bool(memory_region_t)> &callback) const
	{
		memory_region_t last;
		last.mapped = false;
		last.size = 0;
		last.virt_address = is_kernel_space ? (uintptr_t)&_virtual_addr : 0;
		bool do_continue = true;

		auto traversal_callback = [&last, &do_continue, is_kernel_space, &callback](memory_region_t region) {
			uintptr_t start = last.virt_region().end() + 1;
			uintptr_t end = region.virt_address;

			if (start != end)
			{
				memory_region_t free_region = {
				    .virt_address = start,
				    .phys_address = 0,
				    .size = end - start,
				    .mapped = false,
				    .present = false,
				    .allocated = false,
				    .config = {},
				};

				region.config.userspace = !is_kernel_space;
				do_continue = callback(free_region);
			}

			last = region;

			if (!do_continue)
				return false;

			do_continue = callback(region);

			return do_continue;
		};

		if (is_kernel_space)
			m_kernel_memory_map.traverse(traversal_callback);
		else
			m_current_memory_space->userland_map.traverse(traversal_callback);

		if (!do_continue)
			return;

		uintptr_t end = is_kernel_space ? UINTPTR_MAX : (uintptr_t)&_virtual_addr;
		uintptr_t start;

		if (last.mapped)
			start = last.virt_region().end() + 1;
		else
			start = is_kernel_space ? (uintptr_t)&_virtual_addr : 0;

		memory_region_t region = {
		    .virt_address = start,
		    .phys_address = 0,
		    .size = end - start,
		    .mapped = false,
		    .present = false,
		    .allocated = false,
		    .config = {},
		};

		region.config.userspace = !is_kernel_space;

		callback(region);
	}

	void VirtualMemoryManager::traverse_unmapped(bool is_kernel_space, const LibK::function<bool(memory_region_t)> &callback) const
	{
		traverse_all(is_kernel_space, [&callback](memory_region_t region) {
			if (!region.mapped)
				return callback(region);

			return true;
		});
	}

	void VirtualMemoryManager::traverse_mapped(bool is_kernel_space, const LibK::function<bool(memory_region_t)> &callback) const
	{
		traverse_all(is_kernel_space, [&callback](memory_region_t region) {
			if (region.mapped)
				return callback(region);

			return true;
		});
	}

	bool VirtualMemoryManager::in_kernel_space(uintptr_t virt_address) const
	{
		return virt_address >= reinterpret_cast<uintptr_t>(&virt_address) || !m_current_memory_space;
	}
} // namespace Kernel::Memory