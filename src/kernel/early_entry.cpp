#include <crt/icxxabi.hpp>

#include <stdint.h>

#include <arch/Processor.hpp>
#include <panic.hpp>
#include <arch/stack_tracing.hpp>
#include <devices/SerialDevice.hpp>
#include <memory/VirtualMemoryManager.hpp>
#include <arch/smp.hpp>
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
		void entry(multiboot_info_t *);

		typedef void (*func_t)();

		func_t _start_heap_ctors;
		func_t _end_heap_ctors;

		func_t _start_ctors;
		func_t _end_ctors;

		func_t _start_dtors;
		func_t _end_dtors;

		uintptr_t _kernel_start;
		uintptr_t _kernel_end;

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
			// TODO: Fix initialization order
			// LibK::printf_debug_msg("[KERNEL] Kernel image size: %x", size);

			auto start = (uintptr_t)&_kernel_start;
			auto end = (uintptr_t)&_kernel_end;
			uintptr_t size = end - start;

			assert(size <= 0x700000);
			assert(magic == MULTIBOOT_BOOTLOADER_MAGIC);
			assert(multiboot_info);

			// Ensure memory management is setup
			for (func_t *ctor = &_start_heap_ctors; ctor < &_end_heap_ctors; ctor++)
				(*ctor)();

			Heap::init();

			early_preserve_multiboot_info(magic, multiboot_info);

			Memory::VirtualMemoryManager::instance().init(multiboot_info);

			for (func_t *ctor = &_start_ctors; ctor < &_end_ctors; ctor++)
				(*ctor)();

			CPU::Processor::by_id(0).set_memory_space(Memory::VirtualMemoryManager::instance().get_kernel_memory_space());
			CPU::Processor::early_initialize(0);

			CPU::prepare_smp_boot_environment();

			// Get stack traces really early
			CPU::init_stacktracing();

			// Main kernel entry point
			entry(multiboot_info);

			for (func_t *dtor = &_start_dtors; dtor < &_end_dtors; dtor++)
				(*dtor)();

			__cxa_finalize(nullptr);

			for (;;)
				CPU::Processor::halt();
		}
	}
} // namespace Kernel