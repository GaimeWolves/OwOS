#include <filesystem/Ext2FileSystem.hpp>

#include <libk/kcstdio.hpp>

#define EXT2_SIGNATURE 0xef53

namespace Kernel
{
	class Ext2BlockIterator
	{
	public:
		Ext2BlockIterator(uint64_t offset, const Ext2::inode_t &metadata, uint32_t block_size, Ext2FileSystem &filesystem)
		    : m_current_pointer(get_starting_block(offset, block_size, block_size / sizeof (uint32_t)))
		    , m_inode_metadata(metadata)
			, m_block_size(block_size)
			, m_filesystem(filesystem)
		{
			m_triple_region = Memory::VirtualMemoryManager::instance().allocate_region(m_block_size);
			m_double_region = Memory::VirtualMemoryManager::instance().allocate_region(m_block_size);
			m_single_region = Memory::VirtualMemoryManager::instance().allocate_region(m_block_size);

			m_triple_array = reinterpret_cast<uint32_t *>(m_triple_region.virt_address);
			m_double_array = reinterpret_cast<uint32_t *>(m_double_region.virt_address);
			m_single_array = reinterpret_cast<uint32_t *>(m_single_region.virt_address);

			if (m_current_pointer.triply_indirect_block)
			{
				m_filesystem.read(m_inode_metadata.triply_indirect_ptr, 1, m_triple_region.phys_address);
				m_filesystem.read(m_triple_array[m_current_pointer.doubly_indirect_block], 1, m_double_region.phys_address);
				m_filesystem.read(m_double_array[m_current_pointer.singly_indirect_block], 1, m_single_region.phys_address);
				return;
			}

			if (m_current_pointer.doubly_indirect_block)
			{
				m_filesystem.read(m_inode_metadata.doubly_indirect_ptr, 1, m_double_region.phys_address);
				m_filesystem.read(m_double_array[m_current_pointer.singly_indirect_block], 1, m_single_region.phys_address);
				return;
			}

			if (m_current_pointer.singly_indirect_block)
				m_filesystem.read(m_inode_metadata.singly_indirect_ptr, 1, m_single_region.phys_address);
		}

		~Ext2BlockIterator()
		{
			Memory::VirtualMemoryManager::instance().free(m_triple_region);
			Memory::VirtualMemoryManager::instance().free(m_double_region);
			Memory::VirtualMemoryManager::instance().free(m_single_region);
		}

		uint32_t get()
		{
			if (m_current_pointer.triply_indirect_block | m_current_pointer.doubly_indirect_block | m_current_pointer.singly_indirect_block)
				return m_single_array[m_current_pointer.direct_block];

			return m_inode_metadata.direct_ptr[m_current_pointer.direct_block];
		}

		void next()
		{
			auto next_pointer = next_block(m_current_pointer, m_block_size / sizeof(uint32_t));

			if (m_current_pointer.triply_indirect_block != next_pointer.triply_indirect_block)
			{
				m_filesystem.read(m_inode_metadata.triply_indirect_ptr, 1, m_triple_region.phys_address);
				m_filesystem.read(m_triple_array[m_current_pointer.doubly_indirect_block], 1, m_double_region.phys_address);
				m_filesystem.read(m_double_array[m_current_pointer.singly_indirect_block], 1, m_single_region.phys_address);
				return;
			}

			if (m_current_pointer.doubly_indirect_block != next_pointer.doubly_indirect_block)
			{
				m_filesystem.read(m_inode_metadata.doubly_indirect_ptr, 1, m_double_region.phys_address);
				m_filesystem.read(m_double_array[m_current_pointer.singly_indirect_block], 1, m_single_region.phys_address);
				return;
			}

			if (m_current_pointer.singly_indirect_block != next_pointer.singly_indirect_block)
				m_filesystem.read(m_inode_metadata.singly_indirect_ptr, 1, m_single_region.phys_address);

			m_current_pointer = next_pointer;
		}

	private:
		static Ext2::block_pointer_t get_starting_block(size_t offset, uint32_t block_size, size_t inodes_per_block)
		{
			size_t block_number = offset / block_size;

			if (block_number < 12)
				return { block_number, 0, 0, 0 };

			block_number -= 12;

			if (block_number < inodes_per_block)
				return { block_number, 1, 0, 0};

			block_number -= inodes_per_block;

			if (block_number / inodes_per_block < inodes_per_block)
				return { block_number % inodes_per_block, block_number / inodes_per_block, 1, 0};

			block_number -= inodes_per_block * inodes_per_block;

			return { block_number % inodes_per_block, (block_number / inodes_per_block) % inodes_per_block, block_number / (inodes_per_block * inodes_per_block), 1};

		}

		static Ext2::block_pointer_t next_block(Ext2::block_pointer_t block_pointer, size_t inodes_per_block)
		{
			block_pointer.direct_block++;

			if (!block_pointer.triply_indirect_block && !block_pointer.doubly_indirect_block && !block_pointer.singly_indirect_block && block_pointer.direct_block < 12)
			    return block_pointer;

			if ((block_pointer.triply_indirect_block || block_pointer.doubly_indirect_block || block_pointer.singly_indirect_block) && block_pointer.direct_block < inodes_per_block)
				return block_pointer;

			block_pointer.direct_block = 0;
			block_pointer.singly_indirect_block++;

			if (block_pointer.singly_indirect_block < inodes_per_block)
				return block_pointer;

			block_pointer.singly_indirect_block = 0;
			block_pointer.doubly_indirect_block++;

			if (block_pointer.doubly_indirect_block < inodes_per_block)
				return block_pointer;

			block_pointer.doubly_indirect_block = 0;
			block_pointer.triply_indirect_block++;

			assert(block_pointer.triply_indirect_block < 2);

			return block_pointer;

		}

		Ext2::block_pointer_t m_current_pointer;
		const Ext2::inode_t &m_inode_metadata;
		uint32_t m_block_size;
		Ext2FileSystem &m_filesystem;

		Memory::memory_region_t m_triple_region{};
		Memory::memory_region_t m_double_region{};
		Memory::memory_region_t m_single_region{};

		uint32_t *m_triple_array{nullptr};
		uint32_t *m_double_array{nullptr};
		uint32_t *m_single_array{nullptr};
	};

	size_t Ext2File::read(size_t offset, size_t bytes, Memory::memory_region_t region)
	{
		if (!m_inode_metadata_cached)
			read_and_parse_metadata();

		if (offset + bytes > m_size)
			bytes = m_size - offset;

		auto block_iterator = Ext2BlockIterator(offset, m_inode_metadata, m_filesystem->m_block_size, *m_filesystem);

		size_t read_bytes = bytes;

		while (bytes > 0)
		{
			uint32_t block = block_iterator.get();
			m_filesystem->read(block, 1, region.phys_address);
			region.phys_address += m_filesystem->m_block_size;

			if (bytes > m_filesystem->m_block_size)
				bytes -= m_filesystem->m_block_size;
			else
				bytes = 0;

			block_iterator.next();
		}

		return read_bytes;
	}

	size_t Ext2File::write(size_t, size_t, Memory::memory_region_t)
	{
		assert(false);
	}

	bool Ext2File::remove()
	{
		assert(false);
	}

	bool Ext2File::rename(const LibK::string &)
	{
		assert(false);
	}

	LibK::vector<File *> Ext2File::read_directory()
	{
		if (!m_inode_metadata_cached)
			read_and_parse_metadata();

		if (!is_directory())
			return {};

		return m_filesystem->read_directory(m_inode_metadata);
	}

	File *Ext2File::find_file(const LibK::string &)
	{
		if (!is_directory())
			return nullptr;

		assert(false);
	}

	File *Ext2File::make_file(const LibK::string &)
	{
		if (!is_directory())
			return nullptr;

		assert(false);
	}

	FileType Ext2File::from_inode_type(Ext2::InodeType type)
	{
		switch (type)
		{
		case Ext2::InodeType::FIFO: // Todo: Look into FIFO files and UNIX socket files
		case Ext2::InodeType::UnixSocket:
		case Ext2::InodeType::CharDevice:
		case Ext2::InodeType::BlockDevice:
			return FileType::Device;
		case Ext2::InodeType::Directory:
			return FileType::Directory;
		case Ext2::InodeType::RegularFile:
			return FileType::RegularFile;
		case Ext2::InodeType::SoftLink:
			return FileType::SoftLink;
		default:
			return FileType::Unknown;
		}
	}

	void Ext2File::read_and_parse_metadata()
	{
		m_inode_metadata = m_filesystem->read_inode_metadata(m_inode_number);
		m_inode_metadata_cached = true;

		m_size = (uint64_t) m_inode_metadata.size_low | ((uint64_t) m_inode_metadata.size_high << 32);
		m_type = from_inode_type(m_inode_metadata.permission_type.type);
	}

	bool Ext2FileSystem::mount()
	{
		m_file = m_device->open(0);

		m_superblock_region = Memory::VirtualMemoryManager::instance().allocate_region(sizeof (superblock_t));
		m_device->read_blocks(2, LibK::round_up_to_multiple<size_t>(sizeof (superblock_t), m_device->block_size()) / m_device->block_size(), m_superblock_region);
		m_superblock = reinterpret_cast<superblock_t *>(m_superblock_region.virt_address);

		if (m_superblock->signature != EXT2_SIGNATURE)
		{
			Memory::VirtualMemoryManager::instance().free(m_superblock_region);
			m_device->close(m_file);
			return false;
		}

		m_block_size = 1024 << m_superblock->block_size;
		m_sectors_per_block = m_block_size / m_device->block_size();
		m_number_of_block_groups = LibK::ceil_div(m_superblock->block_count, m_superblock->blocks_per_block_group);

		size_t block_descriptor_table_size = LibK::ceil_div(m_number_of_block_groups * sizeof (block_group_descriptor_t), m_device->block_size());
		size_t bg_table_offset = (m_block_size == 1024 ? 2 : 1) * m_sectors_per_block;
		m_bgd_table_region = Memory::VirtualMemoryManager::instance().allocate_region(block_descriptor_table_size * m_device->block_size());
		m_device->read_blocks(bg_table_offset, block_descriptor_table_size, m_bgd_table_region);
		m_bgd_table = reinterpret_cast<block_group_descriptor_t *>(m_bgd_table_region.virt_address);

		root_inode = read_inode_metadata(2);

		return true;
	}

	bool Ext2FileSystem::unmount()
	{
		assert(false);
	}

	LibK::vector<File *> Ext2FileSystem::read_directory()
	{
		return read_directory(root_inode);
	}

	File *Ext2FileSystem::find_file(const LibK::string &)
	{
		assert(false);
	}

	File *Ext2FileSystem::make_file(const LibK::string &)
	{
		assert(false);
	}

	Ext2::inode_t Ext2FileSystem::read_inode_metadata(uint32_t inode)
	{
		uint32_t block_group = (inode - 1) / m_superblock->inodes_per_block_group;
		uint32_t index = (inode - 1) % m_superblock->inodes_per_block_group;
		uint32_t block_in_table = (index * m_superblock->inode_size) / m_block_size;
		uint32_t offset_in_block = (index * m_superblock->inode_size) % m_block_size;
		uint32_t block_to_read = m_bgd_table[block_group].inode_table_start_block + block_in_table;

		auto region = Memory::VirtualMemoryManager::instance().allocate_region(m_block_size);
		m_device->read_blocks(block_to_read * m_sectors_per_block, m_sectors_per_block, region);
		auto *inode_ptr = reinterpret_cast<Ext2::inode_t *>(region.virt_address + offset_in_block);

		Ext2::inode_t inode_data;
		memcpy(&inode_data, inode_ptr, sizeof (Ext2::inode_t));

		Memory::VirtualMemoryManager::instance().free(region);

		return inode_data;
	}

	LibK::vector<File *> Ext2FileSystem::read_directory(const Ext2::inode_t &inode)
	{
		LibK::vector<File *> files;

		if (inode.permission_type.type != Ext2::InodeType::Directory)
			return files;

		auto blocks_used = LibK::ceil_div(inode.size_low, m_block_size);

		// TODO: Extend API to accept physical addresses instead of regions
		auto region = Memory::VirtualMemoryManager::instance().allocate_region(blocks_used * m_block_size);
		m_device->read_blocks(inode.direct_ptr[0] * m_sectors_per_block, m_sectors_per_block, region);
		auto directory_entry = reinterpret_cast<directory_entry_t *>(region.virt_address);

		while (directory_entry->inode != 0)
		{
			if (m_superblock->directory_types)
				files.push_back(new Ext2File(this, directory_entry->inode, directory_entry->name, from_directory_entry_type(directory_entry->type_indicator)));
			else
				files.push_back(new Ext2File(this, directory_entry->inode, directory_entry->name));

			directory_entry = reinterpret_cast<directory_entry_t *>((uintptr_t)directory_entry + directory_entry->size);
		}

		Memory::VirtualMemoryManager::instance().free(region);

		return files;
	}

	size_t Ext2FileSystem::read(size_t block, size_t count, uintptr_t buffer)
	{
		Memory::memory_region_t tmp;
		tmp.phys_address = buffer;
		return m_device->read_blocks(block * m_sectors_per_block, count * m_sectors_per_block, tmp);
	}

	FileType Ext2FileSystem::from_directory_entry_type(DirectoryEntryType type)
	{
		switch (type)
		{
		case DirectoryEntryType::FIFO: // Todo: Look into FIFO files and UNIX socket files
		case DirectoryEntryType::Socket:
		case DirectoryEntryType::CharDevice:
		case DirectoryEntryType::BlockDevice:
			return FileType::Device;
		case DirectoryEntryType::Directory:
			return FileType::Directory;
		case DirectoryEntryType::RegularFile:
			return FileType::RegularFile;
		case DirectoryEntryType::SoftLink:
			return FileType::SoftLink;
		case DirectoryEntryType::Unknown:
			return FileType::Unknown;
		}

		return FileType::Unknown;
	}
}