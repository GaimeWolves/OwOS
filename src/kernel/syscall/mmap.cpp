#include <syscall/syscalls.hpp>

#include "../../userland/libc/sys/mman.h"
#include "../../userland/libc/errno.h"

#include <memory/VirtualMemoryManager.hpp>

namespace Kernel
{
	// https://pubs.opengroup.org/onlinepubs/9699919799/functions/mmap.html
	// For MAP_ANONYMOUS: https://www.man7.org/linux/man-pages/man2/mmap.2.html
	uintptr_t syscall$mmap(SC_mmap_params_t *params)
	{
		// If len is zero, mmap() shall fail and no mapping shall be established.
		if (params->len == 0)
			return -EINVAL;

		// The prot shall be either PROT_NONE or the bitwise-inclusive OR of one or more of the other flags
		if (params->prot > (PROT_EXEC | PROT_READ | PROT_WRITE))
			return -EINVAL;

		// Either MAP_SHARED or MAP_PRIVATE can be specified, but not both.
		if (params->flags & MAP_SHARED && params->flags & MAP_PRIVATE)
			return -EINVAL;

		// Support file mappings later
		if (params->flags != (MAP_PRIVATE | MAP_ANONYMOUS))
			return -ENOTSUP;

		auto config = Memory::mapping_config_t{
		    .readable = static_cast<bool>(params->flags & PROT_READ),
		    .writeable = static_cast<bool>(params->flags & PROT_WRITE),
		    .userspace = true,
		};

		// MAP_ANONYMOUS: The mapping is not backed by any file; its contents are initialized to zero.
		auto region = Memory::VirtualMemoryManager::instance().allocate_region(params->len, config);
		memset(reinterpret_cast<void *>(region.virt_address), 0, region.size);

		if (!region.present)
			return -ENOMEM;

		return region.virt_address;
	}
}