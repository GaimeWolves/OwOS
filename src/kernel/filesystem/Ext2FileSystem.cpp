#include <filesystem/Ext2FileSystem.hpp>

#include "logging/logger.hpp"
#include <libk/kcstdio.hpp>

#include <filesystem/FileSystemCache.hpp>

#define EXT2_SIGNATURE 0xef53
#define EXT2_SUPERBLOCK_OFFSET 1024

#define EXT2_MAX_BYTES_FOR_SYMLINK_DIRECT 60

// TODO: Rewrite this to only use the FS cache to access blocks
namespace Kernel
{

	Ext2::__block_pointer_t Ext2::__block_pointer_t::from_linear_index(uint32_t index, uint32_t inodes_per_block)
	{
		if (index < 12)
			return { index, 0, 0, 0 };

		index -= 12;

		if (index < inodes_per_block)
			return { index, 1, 0, 0 };

		index -= inodes_per_block;

		if (index < inodes_per_block * inodes_per_block)
			return {index % inodes_per_block, index / inodes_per_block, 1, 0 };

		index -= inodes_per_block * inodes_per_block;
		return {
		    index % inodes_per_block,
		    (index / inodes_per_block) % inodes_per_block,
		    index / (inodes_per_block * inodes_per_block),
		    1
		};
	}

	class Ext2BlockIterator
	{
	public:
		Ext2BlockIterator(uint64_t offset, Ext2::inode_t *metadata, uint32_t block_size, Ext2FileSystem *filesystem, BlockDevice *device)
		    : m_current_block_index(offset / block_size)
		    , m_inode_metadata(metadata)
		    , m_inodes_per_block(block_size / sizeof(uint32_t))
		    , m_filesystem(filesystem)
		    , m_device(device)
		{
			m_current_pointer = { 0, 0, 0, 0 };
			auto new_pointer = Ext2::block_pointer_t::from_linear_index(m_current_block_index, m_inodes_per_block);
			update_blocks(new_pointer, false);
			m_current_pointer = new_pointer;
		}

		~Ext2BlockIterator()
		{
			if (m_triple_block)
				FileSystemCache::release(m_triple_block);

			if (m_double_block)
				FileSystemCache::release(m_double_block);

			if (m_single_block)
				FileSystemCache::release(m_single_block);
		}

		void set(uint32_t block)
		{
			if (m_current_block_index < 12)
				m_inode_metadata->direct_ptr[m_current_block_index] = block;

			((uint32_t *)m_single_block->data())[m_current_pointer.direct_block] = block;
		}

		uint32_t get()
		{
			if (m_current_block_index < 12)
				return m_inode_metadata->direct_ptr[m_current_block_index];

			return ((uint32_t *)m_single_block->data())[m_current_pointer.direct_block];
		}

		void next() { next(false); }

		void append_blocks(const LibK::vector<size_t> &blocks)
		{
			for (auto block : blocks)
			{
				next(true);

				if (m_single_block)
					((uint32_t *)m_single_block->data())[m_current_pointer.direct_block] = block;
				else
					m_inode_metadata->direct_ptr[m_current_pointer.direct_block] = block;
			}
		}
	private:
		void next(bool allocate)
		{
			m_current_block_index++;
			auto new_pointer = Ext2::block_pointer_t::from_linear_index(m_current_block_index, m_inodes_per_block);
			update_blocks(new_pointer, allocate);
			m_current_pointer = new_pointer;
		}

		void update_blocks(Ext2::block_pointer_t new_pointer, bool allocate)
		{
			bool triple_block_invalidated = false;
			bool double_block_invalidated = false;
			bool single_block_invalidated = false;

			size_t count = 0;

			if (new_pointer.triply_indirect_block != m_current_pointer.triply_indirect_block)
			{
				triple_block_invalidated = double_block_invalidated = single_block_invalidated = true;
				count = 3;
			}
			else if (new_pointer.doubly_indirect_block != m_current_pointer.doubly_indirect_block)
			{
				double_block_invalidated = single_block_invalidated = true;
				count = 2;
			}
			else if (new_pointer.singly_indirect_block != m_current_pointer.singly_indirect_block)
			{
				single_block_invalidated = true;
				count = 1;
			}

			LibK::vector<size_t> new_blocks;

			if (allocate)
			{
				new_blocks = m_filesystem->allocate_blocks(count);
				assert(new_blocks.size() == count);
			}

			if (triple_block_invalidated)
			{
				if (m_triple_block)
					FileSystemCache::release(m_triple_block);
				m_triple_block = nullptr;

				if (allocate)
					m_inode_metadata->triply_indirect_ptr = new_blocks[2];

				if (new_pointer.triply_indirect_block)
					m_triple_block = FileSystemCache::acquire(m_device, m_inode_metadata->triply_indirect_ptr);
			}

			if (double_block_invalidated)
			{
				if (m_double_block)
					FileSystemCache::release(m_double_block);
				m_double_block = nullptr;

				if (allocate)
				{
					if (m_triple_block)
						((uint32_t *)m_triple_block->data())[new_pointer.doubly_indirect_block] = new_blocks[1];
					else
						m_inode_metadata->doubly_indirect_ptr = new_blocks[1];
				}

				if (m_triple_block || new_pointer.doubly_indirect_block)
				{
					uint32_t block = m_triple_block ? ((uint32_t *)m_triple_block->data())[new_pointer.doubly_indirect_block] : m_inode_metadata->doubly_indirect_ptr;
					m_double_block = FileSystemCache::acquire(m_device, block);
				}
			}

			if (single_block_invalidated)
			{
				if (m_single_block)
					FileSystemCache::release(m_single_block);
				m_single_block = nullptr;

				if (allocate)
				{
					if (m_double_block)
						((uint32_t *)m_double_block->data())[new_pointer.singly_indirect_block] = new_blocks[0];
					else
						m_inode_metadata->singly_indirect_ptr = new_blocks[0];
				}

				if (m_double_block || new_pointer.singly_indirect_block)
				{
					uint32_t block = m_double_block ? ((uint32_t *)m_double_block->data())[new_pointer.singly_indirect_block] : m_inode_metadata->singly_indirect_ptr;
					m_single_block = FileSystemCache::acquire(m_device, block);
				}
			}
		}

		uint32_t m_current_block_index{0};
		Ext2::block_pointer_t m_current_pointer{0, 0, 0, 0};
		Ext2::inode_t *m_inode_metadata{nullptr};
		uint32_t m_inodes_per_block{0};
		Ext2FileSystem *m_filesystem{nullptr};
		BlockDevice *m_device{nullptr};

		fs_block_t *m_triple_block{nullptr};
		fs_block_t *m_double_block{nullptr};
		fs_block_t *m_single_block{nullptr};
	};

	size_t Ext2File::read(size_t offset, size_t bytes, char *buffer)
	{
		if (!m_inode_metadata_cached)
			read_and_parse_metadata();

		if (!is_type(FileType::RegularFile) && !is_type(FileType::SoftLink))
			return 0;

		if (offset + bytes > m_size)
			bytes = m_size - offset;

		if (is_type(FileType::SoftLink) && offset + bytes < EXT2_MAX_BYTES_FOR_SYMLINK_DIRECT)
		{
			// If a symlink is shorter than 60 bytes the data is stored in the inode metadata where the data block pointers would've been.
			char *data = (char *)&m_inode_metadata.direct_ptr[0];
			data += offset;
			memcpy(buffer, data, bytes);
			return bytes;
		}

		auto block_iterator = Ext2BlockIterator(offset, &m_inode_metadata, m_filesystem->m_block_size, m_filesystem, m_filesystem->m_device);

		size_t offset_in_block = offset % m_filesystem->m_block_size;

		size_t read_bytes = 0;

		while (bytes > 0)
		{
			uint32_t block = block_iterator.get();

			size_t to_read = LibK::min<size_t>(m_filesystem->m_block_size, bytes);

			fs_block_t *fs_block = FileSystemCache::acquire(m_filesystem->m_device, block);
			memcpy(buffer, fs_block->data() + offset_in_block, to_read);
			FileSystemCache::release(fs_block);

			bytes -= to_read;
			read_bytes += to_read;
			buffer += to_read;
			offset_in_block = 0;

			block_iterator.next();
		}

		return read_bytes;
	}

	size_t Ext2File::write(size_t offset, size_t bytes, char *buffer)
	{
		if (!m_inode_metadata_cached)
			read_and_parse_metadata();

		if (!is_type(FileType::RegularFile))
			return 0;

		if (offset + bytes > m_size)
		{
			m_lock.lock();
			uint64_t new_size = offset + bytes;
			auto new_block_count = LibK::ceil_div<uint64_t>(new_size, m_filesystem->m_block_size);
			auto old_block_count = LibK::ceil_div<uint64_t>(m_size, m_filesystem->m_block_size);
			uint64_t blocks_to_allocate = new_block_count - old_block_count;

			if (blocks_to_allocate)
			{
				LibK::vector<size_t> blocks = m_filesystem->allocate_blocks(blocks_to_allocate);

				if (blocks.empty())
					return 0;

				auto block_iterator = Ext2BlockIterator(size() == 0 ? 0 : size() - 1, &m_inode_metadata, m_filesystem->m_block_size, m_filesystem, m_filesystem->m_device);

				if (size() == 0)
				{
					// append_blocks() assumes the file already has blocks, so we set the first block manually
					block_iterator.set(blocks[0]);
					blocks.erase(blocks.begin());
				}

				block_iterator.append_blocks(blocks);
			}

			m_size = new_size;
			m_inode_metadata.size_low = m_size & 0xFFFFFFFF;
			m_inode_metadata.size_high = m_size >> 32;

			m_filesystem->write_inode_metadata(inode_number(), m_inode_metadata);
			m_lock.unlock();
		}

		auto block_iterator = Ext2BlockIterator(offset, &m_inode_metadata, m_filesystem->m_block_size, m_filesystem, m_filesystem->m_device);

		size_t offset_in_block = offset % m_filesystem->m_block_size;

		size_t written_bytes = 0;

		while (bytes > 0)
		{
			uint32_t block = block_iterator.get();

			size_t to_write = LibK::min<size_t>(m_filesystem->m_block_size, bytes);

			fs_block_t *fs_block = FileSystemCache::acquire(m_filesystem->m_device, block);
			memcpy(fs_block->data() + offset_in_block, buffer, to_write);
			memset(fs_block->data() + offset_in_block + to_write, 0, m_filesystem->m_block_size - to_write);
			FileSystemCache::release(fs_block);

			bytes -= to_write;
			written_bytes += to_write;
			buffer += to_write;
			offset_in_block = 0;

			block_iterator.next();
		}

		return written_bytes;
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

		if (!is_type(FileType::Directory))
			return {};

		return m_filesystem->read_directory(m_inode_metadata);
	}

	File *Ext2File::find_file(const LibK::string &)
	{
		if (!is_type(FileType::Directory))
			return nullptr;

		assert(false);
	}

	File *Ext2File::make_file(const LibK::string &)
	{
		if (!is_type(FileType::Directory))
			return nullptr;

		assert(false);
	}

	size_t Ext2File::size()
	{
		if (!m_inode_metadata_cached)
			read_and_parse_metadata();

		return m_size;
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
		m_lock.lock();
		m_inode_metadata = m_filesystem->read_inode_metadata(inode_number());
		m_inode_metadata_cached = true;

		m_size = (uint64_t)m_inode_metadata.size_low | ((uint64_t)m_inode_metadata.size_high << 32);
		m_type = from_inode_type(m_inode_metadata.permission_type.type);
		m_lock.unlock();
	}

	bool Ext2FileSystem::mount()
	{
		m_file = m_device->open(0);

		m_superblock_block = FileSystemCache::acquire(m_device, 0);
		m_superblock = (superblock_t *)(m_superblock_block->data() + EXT2_SUPERBLOCK_OFFSET);

		if (m_superblock->signature != EXT2_SIGNATURE)
		{
			FileSystemCache::release(m_superblock_block);
			m_device->close(m_file);
			return false;
		}

		m_block_size = 1024 << m_superblock->block_size;

		// TODO: Handle filesystems with smaller/larger block sizes
		assert(m_block_size == PAGE_SIZE);

		m_sectors_per_block = m_block_size / m_device->block_size();
		m_number_of_block_groups = LibK::ceil_div(m_superblock->block_count, m_superblock->blocks_per_block_group);
		size_t block_descriptor_table_size = LibK::ceil_div(m_number_of_block_groups * sizeof(block_group_descriptor_t), m_device->block_size());

		for (size_t i = 0; i < block_descriptor_table_size; i++)
		{
			// We assume m_block_size == PAGE_SIZE, so the BGD starts at block two
			m_bgd_table_blocks.push_back(FileSystemCache::acquire(m_device, 1 + i));
		}

		for (size_t i = 0; i < m_number_of_block_groups; i++)
		{
			m_block_group_locks.push_back(new Locking::Mutex);
		}

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
		block_group_descriptor_t *bgd = get_block_group_descriptor(block_group);
		uint32_t block_to_read = bgd->inode_table_start_block + block_in_table;

		fs_block_t *table_block = FileSystemCache::acquire(m_device, block_to_read);

		auto *inode_ptr = reinterpret_cast<Ext2::inode_t *>(table_block->data() + offset_in_block);

		Ext2::inode_t inode_data;
		memcpy(&inode_data, inode_ptr, sizeof(Ext2::inode_t));

		FileSystemCache::release(table_block);

		return inode_data;
	}

	void Ext2FileSystem::write_inode_metadata(uint32_t inode_number, const Ext2::inode_t &inode)
	{
		uint32_t block_group = (inode_number - 1) / m_superblock->inodes_per_block_group;
		m_block_group_locks[block_group]->lock();

		uint32_t index = (inode_number - 1) % m_superblock->inodes_per_block_group;
		uint32_t block_in_table = (index * m_superblock->inode_size) / m_block_size;
		uint32_t offset_in_block = (index * m_superblock->inode_size) % m_block_size;
		block_group_descriptor_t *bgd = get_block_group_descriptor(block_group);
		uint32_t block_to_read = bgd->inode_table_start_block + block_in_table;

		fs_block_t *table_block = FileSystemCache::acquire(m_device, block_to_read);
		auto *inode_ptr = reinterpret_cast<Ext2::inode_t *>(table_block->data() + offset_in_block);

		memcpy(inode_ptr, &inode, sizeof(Ext2::inode_t));
		FileSystemCache::sync(table_block);
		FileSystemCache::release(table_block);
	}

	LibK::vector<File *> Ext2FileSystem::read_directory(const Ext2::inode_t &inode)
	{
		LibK::vector<File *> files;

		if (inode.permission_type.type != Ext2::InodeType::Directory)
			return files;

		char *name = static_cast<char *>(kmalloc(UINT8_MAX + 1));

		size_t offset = 0;
		size_t offset_in_block = 0;
		auto block_iterator = Ext2BlockIterator(0, const_cast<Ext2::inode_t *>(&inode), m_block_size, this, m_device);
		fs_block_t *current_block = FileSystemCache::acquire(m_device, block_iterator.get());
		auto *directory_entry = reinterpret_cast<directory_entry_t *>(current_block->data());

		while (true)
		{
			if (directory_entry->inode != 0)
			{
				memcpy(name, directory_entry->name, directory_entry->name_length_low);
				name[directory_entry->name_length_low] = '\0';

				if (m_superblock->directory_types)
					files.push_back(new Ext2File(this, directory_entry->inode, name, from_directory_entry_type(directory_entry->type_indicator)));
				else
					files.push_back(new Ext2File(this, directory_entry->inode, name));
			}

			offset += directory_entry->size;
			offset_in_block += directory_entry->size;

			if (offset >= inode.size_low)
				break;

			if (offset_in_block > m_block_size)
			{
				while (offset_in_block > m_block_size)
					block_iterator.next();

				offset_in_block %= m_block_size;

				FileSystemCache::release(current_block);
				current_block = FileSystemCache::acquire(m_device, block_iterator.get());
			}

			directory_entry = reinterpret_cast<directory_entry_t *>(current_block->data() + offset_in_block);
		}

		kfree(name);
		FileSystemCache::release(current_block);

		return files;
	}

	size_t Ext2FileSystem::read_blocks(size_t block, size_t count, char *buffer)
	{
		size_t read = 0;

		for (size_t i = 0; i < count; i++)
		{
			fs_block_t *fs_block = FileSystemCache::acquire(m_device, block + i);
			memcpy(buffer + i * m_block_size, fs_block->data(), m_block_size);
			FileSystemCache::release(fs_block);
			read += m_block_size;
		}

		return read;
	}

	size_t Ext2FileSystem::write_blocks(size_t block, size_t count, char *buffer)
	{
		size_t written = 0;

		for (size_t i = 0; i < count; i++)
		{
			fs_block_t *fs_block = FileSystemCache::acquire(m_device, block + i);
			memcpy(fs_block->data(), buffer + i * m_block_size, m_block_size);
			FileSystemCache::release(fs_block);
			written += m_block_size;
		}

		return written;
	}

	fs_block_t *Ext2FileSystem::read_block(size_t block)
	{
		return FileSystemCache::acquire(m_device, block);
	}

	Ext2FileSystem::block_group_descriptor_t *Ext2FileSystem::get_block_group_descriptor(size_t index)
	{
		size_t offset = index * sizeof(block_group_descriptor_t);
		size_t block = offset / m_block_size;
		size_t offset_in_block = offset % m_block_size;

		return (block_group_descriptor_t *)(m_bgd_table_blocks[block]->data() + offset_in_block);
	}

	void Ext2FileSystem::sync_block_group_descriptor(size_t index)
	{
		size_t offset = index * sizeof(block_group_descriptor_t);
		size_t block = offset / m_block_size;
		FileSystemCache::sync(m_bgd_table_blocks[block]);
	}

	// ######################################################
	// TODO: Move this into dedicated bitmap helper file
	constexpr size_t bits_per_size_t = sizeof(size_t) * CHAR_BIT;

	static size_t get_size_t(const uint8_t *bitmap, size_t index)
	{
		return ((size_t *)bitmap)[index];
	}

	static bool get_bit(const uint8_t *bitmap, size_t bit)
	{
		return bitmap[bit / CHAR_BIT] & (1 << (bit % CHAR_BIT));
	}

	static void set_bit(uint8_t *bitmap, size_t bit)
	{
		bitmap[bit / CHAR_BIT] |= (1 << (bit % CHAR_BIT));
	}

	[[maybe_unused]] static void clear_bit(uint8_t *bitmap, size_t bit)
	{
		bitmap[bit / CHAR_BIT] &= ~(1 << (bit % CHAR_BIT));
	}
	// ######################################################

	LibK::vector<size_t> Ext2FileSystem::allocate_blocks(size_t count)
	{
		LibK::vector<size_t> blocks;

		m_superblock_lock.lock();

		if (m_superblock->unallocated_blocks < count)
		{
			m_superblock_lock.unlock();
			return blocks;
		}

		m_superblock->unallocated_blocks -= count;
		FileSystemCache::sync(m_superblock_block);

		m_superblock_lock.unlock();

		for (size_t block_group = 0; block_group < m_number_of_block_groups; block_group++)
		{
			if (count == 0)
				break;

			m_block_group_locks[block_group]->lock();

			block_group_descriptor_t *block_group_descriptor = get_block_group_descriptor(block_group);

			size_t allocated_blocks = LibK::min<size_t>(count, block_group_descriptor->unallocated_blocks);

			if (allocated_blocks == 0)
			{
				m_block_group_locks[block_group]->unlock();
				continue;
			}

			block_group_descriptor->unallocated_blocks -= allocated_blocks;
			count -= allocated_blocks;

			size_t current_table_block = 0;
			size_t current_bit_offset = 0;
			fs_block_t *block_table = FileSystemCache::acquire(m_device, block_group_descriptor->block_usage_bitmap_block + current_table_block);
			auto *bitmap = reinterpret_cast<uint8_t *>(block_table->data());
			const size_t blocks_per_block = m_block_size * CHAR_BIT;
			const size_t blocks_in_group = block_group == m_number_of_block_groups - 1 ? m_superblock->block_count % m_superblock->blocks_per_block_group : m_superblock->blocks_per_block_group;

			for (size_t block = 0; block < blocks_in_group; block++)
			{
				if (current_bit_offset % bits_per_size_t == 0 && get_size_t(bitmap, current_bit_offset / bits_per_size_t) == SIZE_MAX)
				{
					// Optimize by skipping fully allocated groups of blocks
					current_bit_offset += bits_per_size_t;
					block += bits_per_size_t - 1;
				}
				else
				{
					if (!get_bit(bitmap, current_bit_offset))
					{
						set_bit(bitmap, current_bit_offset);
						blocks.push_back(block_group * m_superblock->blocks_per_block_group + block);
						allocated_blocks--;

						if (allocated_blocks == 0)
							break;
					}

					current_bit_offset++;
				}

				if (current_bit_offset == blocks_per_block)
				{
					current_bit_offset = 0;
					current_table_block++;
					FileSystemCache::sync(block_table);
					FileSystemCache::release(block_table);
					block_table = FileSystemCache::acquire(m_device, block_group_descriptor->block_usage_bitmap_block + current_table_block);
					bitmap = reinterpret_cast<uint8_t *>(block_table->data());
				}
			}

			FileSystemCache::sync(block_table);
			FileSystemCache::release(block_table);
			sync_block_group_descriptor(block_group);

			m_block_group_locks[block_group]->unlock();
		}

		return blocks;
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
