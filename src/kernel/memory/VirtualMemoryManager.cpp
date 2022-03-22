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

		m_kernel_memory_space->kernel_map.insert(kernel_region);
		m_kernel_memory_space->kernel_map.insert(mapping_region);
	}

	void *VirtualMemoryManager::alloc_kernel_buffer(size_t size, RegionType type, AllocationStategy strategy)
	{
		size = LibK::round_up_to_multiple<size_t>(size, PAGE_SIZE);

		region_t region = find_free_region(size, true, strategy);

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

		region_t region = find_free_region(size, true, strategy);
		auto mem_region = map(phys_addr, region.address, size, false);
		mem_region.is_mmio = true;

		return (void *)mem_region.region.address;
	}

	void VirtualMemoryManager::free(void *ptr)
	{
		auto region = find_region((uintptr_t)ptr);

		assert(region && region->mapped);

		unmap(region->region.address);

		if (region->is_mmio)
			return;

		assert(region->present); // Swapping not yet implemented

		PhysicalMemoryManager::instance().free((void *)region->phys_address, region->region.size);
	}

	void *VirtualMemoryManager::map_physical(uintptr_t phys_addr, size_t size, __unused RegionType type, AllocationStategy strategy)
	{
		uintptr_t start_addr = LibK::round_down_to_multiple<uintptr_t>(phys_addr, PAGE_SIZE);
		uintptr_t start_offset = phys_addr - start_addr;
		size = LibK::round_up_to_multiple<size_t>(size + start_offset, PAGE_SIZE);

		region_t region = find_free_region(size, true, strategy);
		auto mem_region = map(start_addr, region.address, size, false);

		return (void *)(mem_region.region.address + start_offset);
	}

	bool VirtualMemoryManager::try_identity_map(uintptr_t addr, size_t size)
	{
		uintptr_t start_addr = LibK::round_down_to_multiple<uintptr_t>(addr, PAGE_SIZE);
		uintptr_t start_offset = addr - start_addr;
		size = LibK::round_up_to_multiple<size_t>(size + start_offset, PAGE_SIZE);

		region_t this_region{
		    .address = start_addr,
		    .size = size,
		};

		bool is_kernel_space = in_kernel_space(addr);
		bool already_mapped = false;

		traverse_mapped(is_kernel_space, [&](memory_region_t mem_region) {
			if (mem_region.region.overlaps(this_region)) {
				already_mapped = true;
				return false;
			}

			return true;
		});

		if (already_mapped)
			return false;

		map(start_addr, start_addr, size, is_kernel_space);

		return true;
	}

	region_t VirtualMemoryManager::find_free_region(size_t size, bool is_kernel_space, AllocationStategy strategy) const
	{
		assert(size > 0);

		size_t min_size = SIZE_MAX;
		region_t min_region{0, 0};

		traverse_unmapped(is_kernel_space, [&](memory_region_t mem_region) {
			auto region = mem_region.region;

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

		// TODO: Implement more metadata to determine memory (like cache) that can be unmapped
		if (min_region.size == 0)
			panic("Out of kernel virtual memory (OOM) while allocating buffer of size %u", size);

		return min_region;
	}

	const memory_region_t *VirtualMemoryManager::find_region(uintptr_t virtual_addr) const
	{
		auto &tree = virtual_addr > (uintptr_t)&_virtual_addr ? m_current_memory_space->kernel_map : m_current_memory_space->userland_map;

		return tree.find([virtual_addr](memory_region_t region) {
			if (region.region.contains(virtual_addr))
				return 0;
			else if (region.region.address < virtual_addr)
				return -1;
			else
				return 1;
		});
	}

	void VirtualMemoryManager::enumerate(const LibK::function<bool(memory_region_t)> &callback) const
	{
		m_current_memory_space->userland_map.traverse(callback);
		m_current_memory_space->kernel_map.traverse(callback);
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
		auto &tree = is_kernel_space ? m_current_memory_space->kernel_map : m_current_memory_space->userland_map;

		auto region = memory_region_t{
		    .region = {virtual_addr, size},
		    .phys_address = physical_addr,
		    .mapped = true,
		    .present = true,
		    .kernel = !is_user,
		    .is_mmio = false,
		};

		tree.insert(region);

		Arch::map(m_current_memory_space->paging_space, physical_addr, virtual_addr, size, is_user);

		return region;
	}

	void VirtualMemoryManager::unmap(uintptr_t virtual_addr)
	{
		auto is_kernel_space = in_kernel_space(virtual_addr);
		auto &tree = is_kernel_space ? m_current_memory_space->kernel_map : m_current_memory_space->userland_map;

		auto *region = tree.find([virtual_addr](memory_region_t region) {
			if (region.region.contains(virtual_addr))
				return 0;
			else if (region.region.address < virtual_addr)
				return -1;
			else
				return 1;
		});

		if (region)
		{
			Arch::unmap(m_current_memory_space->paging_space, region->region.address, region->region.size);
			tree.remove(*region);
		}
	}

	void VirtualMemoryManager::traverse_all(bool is_kernel_space, const LibK::function<bool(memory_region_t)> &callback) const
	{
		memory_region_t last;
		last.mapped = false;
		last.region.size = 0;
		last.region.address = is_kernel_space ? (uintptr_t)&_virtual_addr : 0;
		bool do_continue = true;

		auto traversal_callback = [&last, &do_continue, is_kernel_space, &callback](memory_region_t region) {
			uintptr_t start = last.region.end() + 1;
			uintptr_t end = region.region.address;

			if (start != end)
			{
				do_continue = callback({
				    .region = {start, end - start},
				    .phys_address = 0,
				    .mapped = false,
				    .present = false,
				    .kernel = is_kernel_space,
				    .is_mmio = false,
				});
			}

			last = region;

			if (!do_continue)
				return false;

			do_continue = callback(region);

			return do_continue;
		};

		if (is_kernel_space)
			m_current_memory_space->kernel_map.traverse(traversal_callback);
		else
			m_current_memory_space->userland_map.traverse(traversal_callback);

		if (!do_continue)
			return;

		uintptr_t end = is_kernel_space ? UINTPTR_MAX : (uintptr_t)&_virtual_addr;
		uintptr_t start;

		if (last.mapped)
			start = last.region.end() + 1;
		else
			start = is_kernel_space ? (uintptr_t)&_virtual_addr : 0;

		callback({
		    .region = {start, end - start},
		    .phys_address = 0,
		    .mapped = false,
		    .present = false,
		    .kernel = is_kernel_space,
		    .is_mmio = false,
		});
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
} // namespace Kernel::Memory