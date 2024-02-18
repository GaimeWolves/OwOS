#include <memory/VirtualMemoryManager.hpp>

#include <common_attributes.h>
#include <memory/PhysicalMemoryManager.hpp>
#include <panic.hpp>
#include <arch/Processor.hpp>

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
		Arch::initialize();

		m_kernel_paging_space = Arch::create_kernel_space();
		Arch::load(m_kernel_paging_space);

		auto kernel_region = Arch::get_kernel_region();
		auto mapping_region = Arch::get_mapping_region();

		m_kernel_memory_map.insert(kernel_region);
		m_kernel_memory_map.insert(mapping_region);

		m_kernel_memory_space = {
		    .paging_space = m_kernel_paging_space,
		    .userland_map = {},
		};

		CPU::Processor::current().set_memory_space(&m_kernel_memory_space);
	}

	memory_region_t VirtualMemoryManager::allocate_region(size_t size, mapping_config_t config)
	{
		auto memory_space = CPU::Processor::current().get_memory_space();
		size = LibK::round_up_to_multiple<size_t>(size, PAGE_SIZE);

		bool is_kernel_space = !config.userspace;

		uintptr_t phys_addr = reinterpret_cast<uintptr_t>(PhysicalMemoryManager::instance().alloc(size, config.bounds.address, config.bounds.end(), config.alignment));

		if (is_kernel_space)
			m_lock.lock();

		region_t region = find_free_region(memory_space, size, is_kernel_space);
		auto mapping = map(memory_space, phys_addr, region.address, size, config);

		if (is_kernel_space)
			m_lock.unlock();

		return mapping;
	}

	memory_region_t VirtualMemoryManager::allocate_region_at(uintptr_t virt_addr, size_t size, mapping_config_t config)
	{
		auto memory_space = CPU::Processor::current().get_memory_space();
		return allocate_region_at_for(memory_space, virt_addr, size, config);
	}

	memory_region_t VirtualMemoryManager::allocate_region_at_for(memory_space_t *memory_space, uintptr_t virt_addr, size_t size, mapping_config_t config)
	{
		uintptr_t address = LibK::round_down_to_multiple<uintptr_t>(virt_addr, PAGE_SIZE);
		size = LibK::round_up_to_multiple<size_t>(size + (virt_addr - address), PAGE_SIZE);
		virt_addr = address;

		uintptr_t phys_addr = reinterpret_cast<uintptr_t>(PhysicalMemoryManager::instance().alloc(size, config.bounds.address, config.bounds.end(), config.alignment));

		bool is_kernel_space = in_kernel_space(virt_addr);

		if (find_region(memory_space, virt_addr))
			return {};

		if (is_kernel_space)
			m_lock.lock();

		auto mapping = map(memory_space, phys_addr, virt_addr, size, config);

		if (is_kernel_space)
			m_lock.unlock();

		return mapping;
	}

	memory_region_t VirtualMemoryManager::map_region(uintptr_t phys_addr, size_t size, mapping_config_t config)
	{
		auto memory_space = CPU::Processor::current().get_memory_space();

		uintptr_t address = LibK::round_down_to_multiple<uintptr_t>(phys_addr, PAGE_SIZE);
		size = LibK::round_up_to_multiple<size_t>(size + (phys_addr - address), PAGE_SIZE);
		phys_addr = address;

		bool is_kernel_space = !config.userspace;

		if (is_kernel_space)
			m_lock.lock();

		region_t region = find_free_region(memory_space, size, is_kernel_space);
		auto mapping = map(memory_space, phys_addr, region.address, size, config);

		if (is_kernel_space)
			m_lock.unlock();

		return mapping;
	}

	memory_region_t VirtualMemoryManager::map_region_at(uintptr_t phys_addr, uintptr_t virt_addr, size_t size, mapping_config_t config)
	{
		auto memory_space = CPU::Processor::current().get_memory_space();

		uintptr_t address = LibK::round_down_to_multiple<uintptr_t>(virt_addr, PAGE_SIZE);
		size = LibK::round_up_to_multiple<size_t>(size + (virt_addr - address), PAGE_SIZE);
		virt_addr = address;

		bool is_kernel_space = in_kernel_space(virt_addr);

		if (find_region(memory_space, virt_addr))
			return {};

		if (is_kernel_space)
			m_lock.lock();

		auto mapping = map(memory_space, phys_addr, virt_addr, size, config);

		if (is_kernel_space)
			m_lock.unlock();

		return mapping;
	}

	void VirtualMemoryManager::free(void *ptr)
	{
		auto memory_space = CPU::Processor::current().get_memory_space();

		auto region = find_region(memory_space, (uintptr_t)ptr);
		assert(region);
		free(*region);
	}

	void VirtualMemoryManager::free(const memory_region_t &region)
	{
		auto memory_space = CPU::Processor::current().get_memory_space();

		assert(region.mapped);

		bool is_kernel_space = in_kernel_space(region.virt_address);
		if (is_kernel_space)
			m_lock.lock();

		unmap(memory_space, region);

		if (is_kernel_space)
			m_lock.unlock();

		if (!region.allocated)
			return;

		assert(region.present); // Swapping not yet implemented

		PhysicalMemoryManager::instance().free((void *)region.phys_address, region.size);
	}

	region_t VirtualMemoryManager::find_free_region(memory_space_t *memory_space, size_t size, bool is_kernel_space) const
	{
		assert(size > 0);

		size_t min_size = SIZE_MAX;
		region_t min_region{0, 0};

		traverse_unmapped(memory_space, is_kernel_space, [&](memory_region_t mem_region) {
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

	const memory_region_t *VirtualMemoryManager::find_region(memory_space_t *memory_space, uintptr_t virtual_addr)
	{
		auto &tree = in_kernel_space(virtual_addr) ? m_kernel_memory_map : memory_space->userland_map;

		bool is_kernel_space = in_kernel_space(virtual_addr);
		if (is_kernel_space)
			m_lock.lock();

		auto region = tree.find([virtual_addr](memory_region_t region) {
			if (region.virt_region().contains(virtual_addr))
				return 0;
			else if (region.virt_address < virtual_addr)
				return -1;
			else
				return 1;
		});

		if (is_kernel_space)
			m_lock.unlock();

		return region;
	}

	void VirtualMemoryManager::enumerate(const LibK::function<bool(memory_region_t)> &callback)
	{
		auto memory_space = CPU::Processor::current().get_memory_space();
		memory_space->userland_map.traverse(callback);

		m_lock.lock();
		m_kernel_memory_map.traverse(callback);
		m_lock.unlock();
	}

	memory_space_t VirtualMemoryManager::create_memory_space()
	{
		auto paging_space = Arch::create_memory_space();

		return {
			.paging_space = paging_space,
			.userland_map = {}
		};
	}

	memory_space_t VirtualMemoryManager::copy_current_memory_space()
	{
		auto current_space = CPU::Processor::current().get_memory_space();
		memory_space_t space = LibK::move(create_memory_space());
		memory_space_t *new_space = &space;

		current_space->userland_map.traverse([current_space, new_space](memory_region_t region) {
			// TODO: copying works for now, but not with file mappings
			auto final_region = VirtualMemoryManager::instance().allocate_region_at_for(new_space, region.virt_address, region.size, region.config);
			auto dest_region = VirtualMemoryManager::instance().map_region(final_region.phys_address, region.size, region.config);

			memcpy(dest_region.virt_region().pointer(), region.virt_region().pointer(), region.size);

			VirtualMemoryManager::instance().unmap(current_space, dest_region);

			return true;
		});

		return LibK::move(space);
	}

	void VirtualMemoryManager::free_current_userspace()
	{
		auto current_space = CPU::Processor::current().get_memory_space();

		LibK::vector<memory_region_t> to_free;

		current_space->userland_map.traverse([&to_free](memory_region_t region) {
			// TODO: this works only for now
			if (region.config.userspace)
				to_free.push_back(region);
			return true;
		});

		for (auto region : to_free)
			VirtualMemoryManager::instance().free(region);
	}

	void VirtualMemoryManager::load_memory_space(memory_space_t *memory_space)
	{
		CPU::Processor::current().enter_critical();
		assert(memory_space);
		CPU::Processor::current().set_memory_space(memory_space);
		Arch::load(memory_space->paging_space);
		CPU::Processor::current().leave_critical();
	}

	memory_region_t VirtualMemoryManager::map(memory_space_t *memory_space, uintptr_t phys_address, uintptr_t virt_address, size_t size, mapping_config_t config)
	{
		auto &tree = in_kernel_space(virt_address) ? m_kernel_memory_map : memory_space->userland_map;

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
		Arch::map(memory_space->paging_space, phys_address, virt_address, size, config);

		return region;
	}

	void VirtualMemoryManager::unmap(memory_space_t *memory_space, const memory_region_t &region)
	{
		auto &tree = in_kernel_space(region.virt_address) ? m_kernel_memory_map : memory_space->userland_map;

		Arch::unmap(memory_space->paging_space, region.virt_address, region.size);
		tree.remove(region);
	}

	bool VirtualMemoryManager::check_free(memory_space_t *memory_space, const region_t &region) const
	{
		bool free = true;

		traverse_mapped(memory_space, in_kernel_space(region.address), [&](memory_region_t mem_region) {
			if (mem_region.virt_region().overlaps(region))
			{
				free = false;
				return false;
			}

			return true;
		});

		return free;
	}

	void VirtualMemoryManager::traverse_all(memory_space_t *memory_space, bool is_kernel_space, const LibK::function<bool(memory_region_t)> &callback) const
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
			memory_space->userland_map.traverse(traversal_callback);

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

	void VirtualMemoryManager::traverse_unmapped(memory_space_t *memory_space, bool is_kernel_space, const LibK::function<bool(memory_region_t)> &callback) const
	{
		traverse_all(memory_space, is_kernel_space, [&callback](memory_region_t region) {
			if (!region.mapped)
				return callback(region);

			return true;
		});
	}

	void VirtualMemoryManager::traverse_mapped(memory_space_t *memory_space, bool is_kernel_space, const LibK::function<bool(memory_region_t)> &callback) const
	{
		traverse_all(memory_space, is_kernel_space, [&callback](memory_region_t region) {
			if (region.mapped)
				return callback(region);

			return true;
		});
	}

	bool VirtualMemoryManager::in_kernel_space(uintptr_t virt_address) const
	{
		return virt_address >= reinterpret_cast<uintptr_t>(&_virtual_addr);
	}
} // namespace Kernel::Memory
