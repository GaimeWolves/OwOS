#include <crt/icxxabi.hpp>

#include <stdint.h>
#include <stddef.h>

#include <arch/processor.hpp>
#include <memory/MemoryManager.hpp>
#include <multiboot.h>

namespace Kernel
{
	extern "C"
	{
		void entry(uint32_t, multiboot_info_t *);

		typedef void (*func_t)();

		func_t _start_heap_ctors;
		func_t _end_heap_ctors;

		func_t _start_ctors;
		func_t _end_ctors;

		func_t _start_dtors;
		func_t _end_dtors;

		__attribute__((noreturn)) void early_entry(uint32_t magic, multiboot_info_t *multiboot_info)
		{
			for (func_t *ctor = &_start_heap_ctors; ctor < &_end_heap_ctors; ctor++)
				(*ctor)();

			Heap::init();
			Memory::MemoryManager::instance().init(multiboot_info);

			for (func_t *ctor = &_start_ctors; ctor < &_end_ctors; ctor++)
				(*ctor)();

			entry(magic, multiboot_info);

			for (func_t *dtor = &_start_ctors; dtor < &_end_dtors; dtor++)
				(*dtor)();

			__cxa_finalize(0);

			for (;;)
				Processor::halt();
		}
	}
}; // namespace Kernel