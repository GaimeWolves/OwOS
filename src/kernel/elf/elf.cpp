#include "elf/elf.hpp"

#include <sys/auxv.h>

#include "elf/definitions.hpp"
#include "filesystem/File.hpp"
#include <arch/Processor.hpp>
#include <filesystem/VirtualFileSystem.hpp>
#include <tty/VirtualConsole.hpp>

namespace Kernel::ELF
{
	static void *s_loader_image_address = nullptr;
	static File *s_loader_file = nullptr;
	static size_t s_loader_image_size = 0;
	static size_t s_loader_entry_offset = 0;

	static bool hasSignature(elf32_ehdr_t *header);
	static void set_up_stack(const char **argv, const char **envp, const char *filename, void *exec_base, void *entry, void *loader_base, thread_t *thread);
	static void load_dynamic_loader_image();
	static uintptr_t map_dynamic_loader();

	static bool hasSignature(elf32_ehdr_t *header)
	{
		return header->e_ident[EI_MAG0] == ELFMAG0 && header->e_ident[EI_MAG1] == ELFMAG1 && header->e_ident[EI_MAG2] == ELFMAG2 && header->e_ident[EI_MAG3] == ELFMAG3;
	}

	static void set_up_stack(const char **argv, const char **envp, const char *filename, void *exec_base, void *entry, void *loader_base, thread_t *thread)
	{
		LibK::vector<char *> env_list;
		LibK::vector<char *> args;

		while (*envp != NULL)
		{
			const char *env = *envp++;
			char *env_copy = reinterpret_cast<char *>(CPU::Processor::thread_push_userspace_data(thread, env, strlen(env) + 1));
			env_list.push_back(env_copy);
		}

		while (*argv != NULL)
		{
			const char *arg = *argv++;
			char *arg_copy = reinterpret_cast<char *>(CPU::Processor::thread_push_userspace_data(thread, arg, strlen(arg) + 1));
			args.push_back(arg_copy);
		}

		char *name = reinterpret_cast<char *>(CPU::Processor::thread_push_userspace_data(thread, filename, strlen(filename) + 1));

		CPU::Processor::thread_align_userspace_stack(thread, alignof(auxv_t));

		auxv_t auxv{
		    .a_type = AT_NULL,
		    .a_un{ .a_val = 0 }
		};

		CPU::Processor::thread_push_userspace_data(thread, auxv);

		auxv.a_type = AT_EXECBASE;
		auxv.a_un.a_ptr = exec_base;
		CPU::Processor::thread_push_userspace_data(thread, auxv);

		auxv.a_type = AT_ENTRY;
		auxv.a_un.a_ptr = entry;
		CPU::Processor::thread_push_userspace_data(thread, auxv);

		auxv.a_type = AT_EXECFN;
		auxv.a_un.a_ptr = name;
		CPU::Processor::thread_push_userspace_data(thread, auxv);

		auxv.a_type = AT_PAGESZ;
		auxv.a_un.a_val = PAGE_SIZE;
		CPU::Processor::thread_push_userspace_data(thread, auxv);

		auxv.a_type = AT_BASE;
		auxv.a_un.a_ptr = loader_base;
		CPU::Processor::thread_push_userspace_data(thread, auxv);

		CPU::Processor::thread_push_userspace_data(thread, nullptr);
		for (int i = (int)env_list.size() - 1; i >= 0; i--)
			CPU::Processor::thread_push_userspace_data(thread, env_list[i]);

		CPU::Processor::thread_push_userspace_data(thread, nullptr);
		for (int i = (int)args.size() - 1; i >= 0; i--)
			CPU::Processor::thread_push_userspace_data(thread, args[i]);
		CPU::Processor::thread_push_userspace_data(thread, (int)args.size());
	}

	static void load_dynamic_loader_image()
	{
		if (s_loader_image_address)
			return;

		s_loader_file = VirtualFileSystem::instance().find_by_path("/lib/ld-owos.so");
		auto region = Memory::VirtualMemoryManager::instance().allocate_region(s_loader_file->size());
		s_loader_file->read(0, s_loader_file->size(), reinterpret_cast<char *>(region.virt_address));
		auto header = static_cast<elf32_ehdr_t *>((void *)region.virt_address);

		assert(hasSignature(header)); // TODO: Error handling

		for (int i = 0; i < header->e_phnum; i++)
		{
			auto *pheader = (elf32_phdr_t *)(region.virt_address + header->e_phoff + header->e_phentsize * i);
			if (pheader->p_type == PT_LOAD)
			{
				s_loader_image_size = LibK::max<size_t>(s_loader_image_size, pheader->p_vaddr + pheader->p_memsz);
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

	thread_t *load(Process *parent_process, File *file, const char **argv, const char **envp, bool is_exec_syscall)
	{
		auto region = Memory::VirtualMemoryManager::instance().allocate_region(file->size());
		file->read(0, file->size(), reinterpret_cast<char *>(region.virt_address));
		auto header = static_cast<elf32_ehdr_t *>((void *)region.virt_address);

		assert(hasSignature(header)); // TODO: Error handling

		uintptr_t entry = header->e_entry;
		uintptr_t offset = 0;

		bool is_dynamic = header->e_type == ET_DYN || header->e_type == ET_REL;

		if (is_dynamic)
		{
			offset = 0x55555000; // TODO: ASLR

			load_dynamic_loader_image();
			assert(s_loader_image_address);

			entry = 0x88888000 + s_loader_entry_offset; // Ehh
		}

		auto old_memory_space = CPU::Processor::current().get_memory_space();
		auto &memory_space = parent_process->get_memory_space();
		CPU::Processor::current().enter_critical();
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

		uintptr_t loader_base = 0;

		if (is_dynamic)
			loader_base = map_dynamic_loader();

		thread_t *thread = parent_process->get_thread_by_index(0);
		CPU::Processor::initialize_userspace_thread(thread, entry, parent_process->get_memory_space());

		set_up_stack(argv, envp, file->name().c_str(), reinterpret_cast<void *>(offset), reinterpret_cast<void *>(offset + header->e_entry), reinterpret_cast<void *>(loader_base), thread);

		Memory::VirtualMemoryManager::load_memory_space(old_memory_space);
		CPU::Processor::current().leave_critical();

		if (!is_exec_syscall)
			parent_process->start_thread(0);

		return thread;
	}

	bool is_executable(File *file)
	{
		if (file->size() < sizeof(elf32_ehdr_t))
			return false;

		auto region = Memory::VirtualMemoryManager::instance().allocate_region(sizeof(elf32_ehdr_t));
		file->read(0, sizeof(elf32_ehdr_t), reinterpret_cast<char *>(region.virt_address));
		auto header = (elf32_ehdr_t *)region.virt_address;

		bool valid = hasSignature(header) && (header->e_type == ET_DYN || header->e_type == ET_REL || header->e_type == ET_EXEC);

		Memory::VirtualMemoryManager::instance().free(region);

		return valid;
	}
}
