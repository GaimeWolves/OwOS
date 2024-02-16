#include <syscall/syscalls.hpp>

#include <libk/ErrorOr.hpp>

#include <arch/Processor.hpp>
#include <filesystem/VirtualFileSystem.hpp>
#include <filesystem/File.hpp>

#include "../../userland/libc/signal.h"

namespace Kernel
{
	uintptr_t syscall$sigreturn(thread_registers_t *original_regs, CPU::interrupt_frame_t *frame)
	{
		auto &core = CPU::Processor::current();
		auto process = core.get_current_thread()->parent_process;
		assert(process);

		return CPU::Processor::do_sigreturn(core.get_current_thread(), original_regs, frame);;
	}
}