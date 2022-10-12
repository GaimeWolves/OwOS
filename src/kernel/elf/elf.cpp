#include "elf/elf.hpp"

#include "libk/kcstdio.hpp"

#include "elf/definitions.hpp"
#include "filesystem/File.hpp"
#include <arch/Processor.hpp>

namespace Kernel::ELF
{
	static bool hasSignature(elf32_ehdr_t *header);

	static bool hasSignature(elf32_ehdr_t *header)
	{
		return header->e_ident[EI_MAG0] == ELFMAG0 && header->e_ident[EI_MAG1] == ELFMAG1 && header->e_ident[EI_MAG2] == ELFMAG2 && header->e_ident[EI_MAG3] == ELFMAG3;
	}

	Process load(File *file)
	{
		auto region = Memory::VirtualMemoryManager::instance().allocate_region(sizeof(elf32_ehdr_t));
		file->read(0, sizeof(elf32_ehdr_t), region);
		auto header = static_cast<elf32_ehdr_t *>((void *)region.virt_address);

		assert(hasSignature(header)); // TODO: Error handling

		size_t physical_header_size = header->e_phentsize * header->e_phnum;
		auto header_region = Memory::VirtualMemoryManager::instance().allocate_region(physical_header_size);
		file->read(header->e_phoff, physical_header_size, header_region);

		uintptr_t offset = 0x88888000; // TODO: ASLR

		LibK::vector<Memory::memory_region_t> regions;

		for (int i = 0; i < header->e_phnum; i++)
		{
			auto *pheader = (elf32_phdr_t *)(header_region.virt_address + header->e_phoff + header->e_phentsize * i); // TODO: in VFS correctly implement reading from an offset;
			if (pheader->p_type == PT_LOAD)
			{
				// NOTE: This assumes page aligned PT_LOAD entries
				auto kernel_region = Memory::VirtualMemoryManager::instance().allocate_region(pheader->p_memsz);
				file->read(pheader->p_offset, pheader->p_filesz, kernel_region);
				memset((void *)(kernel_region.virt_address + pheader->p_filesz), 0, pheader->p_memsz - pheader->p_filesz);
				regions.push_back(LibK::move(kernel_region));
			}
		}

		auto old_memory_space = CPU::Processor::current().get_memory_space();
		auto memory_space = Memory::VirtualMemoryManager::create_memory_space();
		Memory::VirtualMemoryManager::load_memory_space(&memory_space);

		// TODO: Fix paging/memory management code to not map already present pages in the kernel when on a different memory space
		int idx = 0;
		for (int i = 0; i < header->e_phnum; i++)
		{
			auto *pheader = (elf32_phdr_t *)(header_region.virt_address + header->e_phoff + header->e_phentsize * i);
			if (pheader->p_type == PT_LOAD)
			{
				// NOTE: This assumes page aligned PT_LOAD entries
				auto mapping_conf = Memory::mapping_config_t();
				mapping_conf.userspace = true;
				//mapping_conf.writeable = pheader->p_flags & PF_W;
				//mapping_conf.readable = pheader->p_flags & PF_R;
				auto phys_region = Memory::VirtualMemoryManager::instance().allocate_region_at(offset + pheader->p_vaddr, pheader->p_memsz, mapping_conf);
				memcpy((void *)phys_region.virt_address, (void *)(regions[idx++].virt_address + pheader->p_offset), pheader->p_memsz);
			}
		}

		auto process = Process(header->e_entry + offset, memory_space);

		Memory::VirtualMemoryManager::load_memory_space(old_memory_space);

		return process;
	}
}