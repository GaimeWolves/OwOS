#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H 1

#include <stddef.h>
#include <stdint.h>

#include <memory/Pages.hpp>
#include <memory/MultibootMap.hpp>

#include <multiboot.h>

namespace Kernel::Memory
{

    class MemoryManager
    {
    public:
        static MemoryManager &instance()
        {
            static MemoryManager *instance{nullptr};

            if (!instance)
                instance = new MemoryManager();

            return *instance;
        }

        MemoryManager(MemoryManager &) = delete;
        void operator=(const MemoryManager &) = delete;

        void init(multiboot_info_t *&multiboot_info);

        void *map_mmio_region(uintptr_t physical_addr, size_t size);

        void unmap_region(uintptr_t virtual_addr, size_t size);

    private:
        MemoryManager();
        ~MemoryManager() = default;

        void map_page_directory();
        void map_kernel();

        void preserve_multiboot_info(multiboot_info_t *&multiboot_info);

        void set_page_directory(page_directory_t *new_pd);

        page_directory_entry_t create_pde(size_t pd_index, bool is_user, bool is_writeable, bool disable_cache);
        page_table_entry_t create_pte(uintptr_t page_address, page_directory_entry_t &pde, bool is_user, bool is_writeable, bool disable_cache);

        void map(uintptr_t physical_address, uintptr_t virtual_address, bool is_user, bool is_writeable, bool disable_cache, bool invalidate);
        void map_range(uintptr_t physical_address, size_t size, uintptr_t virtual_address, bool is_user, bool is_writeable, bool disable_cache, bool invalidate);

        void unmap(uintptr_t virtual_address, bool invalidate);
        void unmap_range(uintptr_t virtual_address, size_t size, bool invalidate);

        page_directory_t *m_current_pd;
        page_directory_t *m_kernel_pd;

        // As page 0xFFFFF000 - 0xFFFFFFFF is reserved for the
        // page directory lookup we cannot access the mapping table
        // that is pointed to by the last PDE in the page directory
        // with the page table lookup from 0xFFC00000 so we save
        // the values here.
        page_table_t *m_current_mapping_table;
        page_table_t *m_kernel_mapping_table;

        MultibootMap m_memory_map;
    };

}; // namespace Kernel::Memory

#endif // MEMORY_MANAGER_H