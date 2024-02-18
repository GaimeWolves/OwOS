#include <syscall/syscalls.hpp>

#include <libk/ErrorOr.hpp>

#include <arch/Processor.hpp>
#include <filesystem/VirtualFileSystem.hpp>
#include <filesystem/File.hpp>

#include "../../userland/libc/signal.h"

namespace Kernel
{
	uintptr_t syscall$sigaction(int signal, const struct sigaction *act, struct sigaction *oact)
	{
		auto process = CPU::Processor::current().get_current_thread()->parent_process;
		assert(process);

		LibK::ErrorOr<uintptr_t> ret(ESUCCESS);

		if (act)
			ret = process->set_signal_handler((int8_t)signal, (uintptr_t)act->sa_handler);

		if (oact)
		{
			oact->sa_flags = 0;
			oact->sa_mask = 0;
			oact->sa_handler = ret.has_error() ? nullptr : (void (*)(int))ret.data();
		}

		if (ret.has_error())
			return -ret.error();

		return -ESUCCESS;
	}
}
