#include <crt/icxxabi.hpp>

#include <stdint.h>

#include <arch/processor.hpp>
#include <panic.hpp>
#include <arch/stack_tracing.hpp>
#include <devices/SerialPort.hpp>
#include <memory/VirtualMemoryManager.hpp>
#include <multiboot.h>
#include <common_attributes.h>

#include <libk/kcmalloc.hpp>
#include <libk/kcstring.hpp>

extern "C"
{
	extern uintptr_t _virtual_addr;
}

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

		static void early_preserve_multiboot_info(uint32_t magic, multiboot_info_t *&multiboot_info);

		// Move multiboot info structures into kernel space (kernel heap)
		static void early_preserve_multiboot_info(uint32_t magic, multiboot_info_t *&multiboot_info)
		{
			assert(magic == MULTIBOOT_BOOTLOADER_MAGIC);
			assert(multiboot_info);

			multiboot_info = (multiboot_info_t *)((uintptr_t)multiboot_info + (uintptr_t)&_virtual_addr);
			auto *new_multiboot_info = (multiboot_info_t *)kmalloc(sizeof(multiboot_info_t));
			memmove(new_multiboot_info, multiboot_info, sizeof(multiboot_info_t));
			multiboot_info = new_multiboot_info;

			assert(multiboot_info->flags & MULTIBOOT_INFO_MEM_MAP);

			multiboot_info->mmap_addr += (uint32_t)&_virtual_addr;
			auto *new_mmap_addr = (multiboot_mmap_entry_t *)kmalloc(multiboot_info->mmap_length);
			memmove(new_mmap_addr, (void *)multiboot_info->mmap_addr, multiboot_info->mmap_length);
			multiboot_info->mmap_addr = (uint32_t)new_mmap_addr;
		}

		__noreturn void early_entry(uint32_t magic, multiboot_info_t *multiboot_info)
		{
			// Ensure memory management is setup
			for (func_t *ctor = &_start_heap_ctors; ctor < &_end_heap_ctors; ctor++)
				(*ctor)();

			Heap::init();

			early_preserve_multiboot_info(magic, multiboot_info);

			Memory::VirtualMemoryManager::instance().init(multiboot_info);

			for (func_t *ctor = &_start_ctors; ctor < &_end_ctors; ctor++)
				(*ctor)();

			// Get debug output really early
			Devices::SerialPort::init();

			// Get stack traces really early
			Processor::init_stacktracing();

			// Main kernel entry point
			entry(magic, multiboot_info);

			for (func_t *dtor = &_start_dtors; dtor < &_end_dtors; dtor++)
				(*dtor)();

			__cxa_finalize(nullptr);

			for (;;)
				Processor::halt();
		}
	}
} // namespace Kernel