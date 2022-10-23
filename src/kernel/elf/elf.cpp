#include "elf/elf.hpp"

#include <sys/arch/i386/auxv.h>

#include "libk/kcstdio.hpp"

#include "elf/definitions.hpp"
#include "filesystem/File.hpp"
#include <devices/StdioDevice.hpp>
#include <arch/Processor.hpp>

namespace Kernel::ELF
{
	static bool hasSignature(elf32_ehdr_t *header);
	static void set_up_stack(const char *filename, void *exec_base, void *entry, void *loader_base, Process *process);

	static bool hasSignature(elf32_ehdr_t *header)
	{
		return header->e_ident[EI_MAG0] == ELFMAG0 && header->e_ident[EI_MAG1] == ELFMAG1 && header->e_ident[EI_MAG2] == ELFMAG2 && header->e_ident[EI_MAG3] == ELFMAG3;
	}

	// TODO: This is an early implementation of stack setup
	static void set_up_stack(const char *filename, void *exec_base, void *entry, void *loader_base, Process *process)
	{
		// Arguments for testing purposes
		static const char *arg2_data = "test";
		static const char *env1_data = "HOME=/usr/";
		static const char *env2_data = "PATH=/bin/";

		auto main_thread = process->get_thread_by_index(0);

		char *env2 = reinterpret_cast<char *>(CPU::Processor::thread_push_userspace_data(main_thread, env2_data, strlen(env2_data) + 1));
		char *env1 = reinterpret_cast<char *>(CPU::Processor::thread_push_userspace_data(main_thread, env1_data, strlen(env1_data) + 1));
		char *arg2 = reinterpret_cast<char *>(CPU::Processor::thread_push_userspace_data(main_thread, arg2_data, strlen(arg2_data) + 1));
		char *arg1 = reinterpret_cast<char *>(CPU::Processor::thread_push_userspace_data(main_thread, filename, strlen(filename) + 1));

		auxv_t auxv{
		    .a_type = AT_NULL,
		    .a_un{ .a_val = 0 }
		};

		CPU::Processor::thread_push_userspace_data(main_thread, auxv);

		auxv.a_type = AT_EXECBASE;
		auxv.a_un.a_ptr = exec_base;
		CPU::Processor::thread_push_userspace_data(main_thread, auxv);

		auxv.a_type = AT_ENTRY;
		auxv.a_un.a_ptr = entry;
		CPU::Processor::thread_push_userspace_data(main_thread, auxv);

		auxv.a_type = AT_EXECFD;
		auxv.a_un.a_val = 4;
		CPU::Processor::thread_push_userspace_data(main_thread, auxv);

		auxv.a_type = AT_EXECFN;
		auxv.a_un.a_ptr = arg1;
		CPU::Processor::thread_push_userspace_data(main_thread, auxv);

		auxv.a_type = AT_PAGESZ;
		auxv.a_un.a_val = PAGE_SIZE;
		CPU::Processor::thread_push_userspace_data(main_thread, auxv);

		auxv.a_type = AT_BASE;
		auxv.a_un.a_ptr = loader_base;
		CPU::Processor::thread_push_userspace_data(main_thread, auxv);

		CPU::Processor::thread_push_userspace_data(main_thread, nullptr);
		CPU::Processor::thread_push_userspace_data(main_thread, env2);
		CPU::Processor::thread_push_userspace_data(main_thread, env1);

		CPU::Processor::thread_push_userspace_data(main_thread, nullptr);
		CPU::Processor::thread_push_userspace_data(main_thread, arg2);
		CPU::Processor::thread_push_userspace_data(main_thread, arg1);
		CPU::Processor::thread_push_userspace_data(main_thread, (int)2);
	}

	Process *load(File *file)
	{
		auto region = Memory::VirtualMemoryManager::instance().allocate_region(0x6000);
		file->read(0, 0x6000, region); // TODO: in VFS correctly implement reading from an offset;
		auto header = static_cast<elf32_ehdr_t *>((void *)region.virt_address);

		assert(hasSignature(header)); // TODO: Error handling

		uintptr_t offset = 0;

		if (header->e_type == ET_DYN || header->e_type == ET_REL)
			offset = 0x55555000; // TODO: ASLR

		auto process = new Process(header->e_entry + offset);

		// Temporary stdio device files
		process->add_file(StdioDevice::get()->open(O_WRONLY));
		process->add_file(StdioDevice::get()->open(O_RDONLY));
		process->add_file(StdioDevice::get()->open(O_WRONLY));

		auto fd = file->open(O_RDWR);
		process->add_file(LibK::move(fd));

		auto old_memory_space = CPU::Processor::current().get_memory_space();
		auto &memory_space = process->get_memory_space();
		Memory::VirtualMemoryManager::load_memory_space(&memory_space);

		for (int i = 0; i < header->e_phnum; i++)
		{
			auto *pheader = (elf32_phdr_t *)(region.virt_address + header->e_phoff + header->e_phentsize * i);
			if (pheader->p_type == PT_LOAD)
			{
				// NOTE: This assumes page aligned PT_LOAD entries
				auto mapping_conf = Memory::mapping_config_t();
				mapping_conf.userspace = true;
				//mapping_conf.writeable = pheader->p_flags & PF_W;
				//mapping_conf.readable = pheader->p_flags & PF_R;
				auto phys_region = Memory::VirtualMemoryManager::instance().allocate_region_at(offset + pheader->p_vaddr - pheader->p_offset % pheader->p_align, pheader->p_memsz + pheader->p_offset % pheader->p_align, mapping_conf);
				memcpy((void *)(phys_region.virt_address + pheader->p_offset % pheader->p_align), (void *)(region.virt_address + pheader->p_offset), pheader->p_filesz);
				memset((void *)(phys_region.virt_address + pheader->p_offset % pheader->p_align + pheader->p_filesz), 0, pheader->p_memsz - pheader->p_filesz);
			}
		}

		set_up_stack(file->name().c_str(), reinterpret_cast<void *>(offset), reinterpret_cast<void *>(offset + header->e_entry), reinterpret_cast<void *>(offset), process);

		Memory::VirtualMemoryManager::load_memory_space(old_memory_space);

		return process;
	}
}