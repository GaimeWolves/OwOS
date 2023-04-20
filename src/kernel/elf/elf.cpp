#include "elf/elf.hpp"

#include <sys/arch/i386/auxv.h>

#include "elf/definitions.hpp"
#include "filesystem/File.hpp"
#include <arch/Processor.hpp>
#include <filesystem/VirtualFileSystem.hpp>
#include <tty/TTY.hpp>

namespace Kernel::ELF
{
	static void *s_loader_image_address = nullptr;
	static File *s_loader_file = nullptr;
	static size_t s_loader_image_size = 0;
	static size_t s_loader_entry_offset = 0;

	static bool hasSignature(elf32_ehdr_t *header);
	static void set_up_stack(const char *filepath, const char *filename, void *exec_base, void *entry, void *loader_base, Process *process);
	static void load_dynamic_loader_image();
	static uintptr_t map_dynamic_loader();

	static bool hasSignature(elf32_ehdr_t *header)
	{
		return header->e_ident[EI_MAG0] == ELFMAG0 && header->e_ident[EI_MAG1] == ELFMAG1 && header->e_ident[EI_MAG2] == ELFMAG2 && header->e_ident[EI_MAG3] == ELFMAG3;
	}

	// TODO: This is an early implementation of stack setup
	static void set_up_stack(const char *filepath, const char *filename, void *exec_base, void *entry, void *loader_base, Process *process)
	{
		// Arguments for testing purposes
		static const char *env1_data = "HOME=/usr/";
		static const char *env2_data = "PATH=/bin/";

		auto main_thread = process->get_thread_by_index(0);

		char *env2 = reinterpret_cast<char *>(CPU::Processor::thread_push_userspace_data(main_thread, env2_data, strlen(env2_data) + 1));
		char *env1 = reinterpret_cast<char *>(CPU::Processor::thread_push_userspace_data(main_thread, env1_data, strlen(env1_data) + 1));
		char *arg1 = reinterpret_cast<char *>(CPU::Processor::thread_push_userspace_data(main_thread, filepath, strlen(filepath) + 1));
		char *name = reinterpret_cast<char *>(CPU::Processor::thread_push_userspace_data(main_thread, filename, strlen(filename) + 1));

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
		auxv.a_un.a_ptr = name;
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
		CPU::Processor::thread_push_userspace_data(main_thread, arg1);
		CPU::Processor::thread_push_userspace_data(main_thread, (int)2);
	}

	static void load_dynamic_loader_image()
	{
		if (s_loader_image_address)
			return;

		s_loader_file = VirtualFileSystem::instance().find_by_path("/lib/ld-owos.so");
		auto region = Memory::VirtualMemoryManager::instance().allocate_region(s_loader_file->size());
		s_loader_file->read(0, s_loader_file->size(), region);
		auto header = static_cast<elf32_ehdr_t *>((void *)region.virt_address);

		assert(hasSignature(header)); // TODO: Error handling

		for (int i = 0; i < header->e_phnum; i++)
		{
			auto *pheader = (elf32_phdr_t *)(region.virt_address + header->e_phoff + header->e_phentsize * i);
			if (pheader->p_type == PT_LOAD)
			{
				s_loader_image_size = LibK::max(s_loader_image_size, pheader->p_vaddr + pheader->p_memsz);
			}
		}

		s_loader_entry_offset = header->e_entry;

		auto image_region = Memory::VirtualMemoryManager::instance().allocate_region(s_loader_image_size);
		s_loader_image_address = image_region.virt_region().pointer();
		assert(s_loader_image_address);

		for (int i = 0; i < header->e_phnum; i++)
		{
			auto *pheader = (elf32_phdr_t *)(region.virt_address + header->e_phoff + header->e_phentsize * i);
			if (pheader->p_type == PT_LOAD)
			{
				memcpy((void *)(image_region.virt_address + pheader->p_vaddr), (void *)(region.virt_address + pheader->p_offset), pheader->p_filesz);
				memset((void *)(image_region.virt_address + pheader->p_vaddr + pheader->p_filesz), 0, pheader->p_memsz - pheader->p_filesz);
			}
		}
	}

	static uintptr_t map_dynamic_loader()
	{
		assert(s_loader_image_address);

		uintptr_t offset = 0x88888000; // TODO: ASLR
		auto mapping_conf = Memory::mapping_config_t();
		mapping_conf.userspace = true;
		auto region = Memory::VirtualMemoryManager::instance().allocate_region_at(offset, s_loader_image_size, mapping_conf);
		assert(region.present);
		memcpy((void *)offset, s_loader_image_address, s_loader_image_size);
		return offset;
	}

	Process *load(const char *filepath)
	{
		File *file = VirtualFileSystem::instance().find_by_path(filepath);
		auto region = Memory::VirtualMemoryManager::instance().allocate_region(file->size());
		file->read(0, file->size(), region);
		auto header = static_cast<elf32_ehdr_t *>((void *)region.virt_address);

		assert(hasSignature(header)); // TODO: Error handling

		uintptr_t offset = 0;

		if (header->e_type == ET_DYN || header->e_type == ET_REL)
			offset = 0x55555000; // TODO: ASLR

		load_dynamic_loader_image();
		assert(s_loader_image_address);

		uintptr_t entry = 0x88888000 + s_loader_entry_offset; // Ehh

		auto process = new Process(entry);

		// Temporary stdio device files
		process->add_file(TTY::get_tty()->open(O_RDONLY));
		process->add_file(TTY::get_tty()->open(O_WRONLY));
		process->add_file(TTY::get_tty()->open(O_WRONLY));

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

		uintptr_t loader_base = map_dynamic_loader();

		set_up_stack(filepath, file->name().c_str(), reinterpret_cast<void *>(offset), reinterpret_cast<void *>(offset + header->e_entry), reinterpret_cast<void *>(loader_base), process);

		Memory::VirtualMemoryManager::load_memory_space(old_memory_space);

		return process;
	}
}