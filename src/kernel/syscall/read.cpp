#include <syscall/syscalls.hpp>

#include <arch/Processor.hpp>
#include <arch/memory.hpp>
#include <processes/Process.hpp>

namespace Kernel
{
	uintptr_t syscall$read(int fd, void *buf, size_t count)
	{
		auto process = CPU::Processor::current().get_current_thread()->parent_process;
		assert(process);
		auto &file = process->get_file_by_index(fd);
		Memory::memory_region_t region;
		region.virt_address = reinterpret_cast<uintptr_t>(buf);
		region.phys_address = Memory::Arch::as_physical(region.virt_address);
		region.size = count;
		return file.read(count, region);
	}
}