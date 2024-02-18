#pragma once

#include <filesystem/FileSystem.hpp>
#include <filesystem/FileSystemCache.hpp>
#include <locking/Mutex.hpp>
#include <devices/BlockDevice.hpp>
#include <memory/VirtualMemoryManager.hpp>

namespace Kernel
{
	class Ext2FileSystem;
	class Ext2File;

	namespace Ext2
	{
		typedef struct __block_pointer_t
		{
			uint32_t direct_block;
			uint32_t singly_indirect_block;
			uint32_t doubly_indirect_block;
			uint32_t triply_indirect_block;

			static __block_pointer_t from_linear_index(uint32_t index, uint32_t inodes_per_block);
		} block_pointer_t;

		enum class InodeType : uint16_t
		{
			FIFO = 0x1,
			CharDevice = 0x2,
			Directory = 0x4,
			BlockDevice = 0x6,
			RegularFile = 0x8,
			SoftLink = 0xA,
			UnixSocket = 0xC,
		};

		typedef struct __inode_t
		{
			struct
			{
				uint16_t other_execute : 1;
				uint16_t other_write : 1;
				uint16_t other_read : 1;
				uint16_t group_execute : 1;
				uint16_t group_write : 1;
				uint16_t group_read : 1;
				uint16_t user_execute : 1;
				uint16_t user_write : 1;
				uint16_t user_read : 1;
				uint16_t sticky_bit : 1;
				uint16_t set_gid : 1;
				uint16_t set_uid : 1;
				InodeType type : 4;
			} permission_type;

			uint16_t uid;
			uint32_t size_low;
			uint32_t atime;
			uint32_t ctime;
			uint32_t mtime;
			uint32_t dtime;
			uint16_t gid;
			uint16_t hardlink_count;
			uint32_t disk_sector_count;

			struct
			{
				uint32_t secure_deletion : 1;
				uint32_t keep_copy_on_deletion : 1;
				uint32_t compression : 1;
				uint32_t synchronous_updates : 1;
				uint32_t immutable : 1;
				uint32_t append_only : 1;
				uint32_t not_included_in_dump : 1;
				uint32_t ignore_atime : 1, : 8;
				uint32_t hash_indexed_directory : 1;
				uint32_t afs_directory : 1;
				uint32_t journal_file_data : 1, : 13;
			} flags;

			uint32_t os_value1;

			uint32_t direct_ptr[12];
			uint32_t singly_indirect_ptr;
			uint32_t doubly_indirect_ptr;
			uint32_t triply_indirect_ptr;

			uint32_t generation_number;
			uint32_t file_acl;

			union
			{
				uint32_t size_high;
				uint32_t directory_acl;
			};

			uint32_t fragment_address;
			uint32_t os_value2;
		} __packed inode_t;
	}

	class Ext2File final : public File
	{
		friend Ext2FileSystem;

	public:
		explicit Ext2File(Ext2FileSystem *file_system, uint32_t inode_number, char *name)
		    : m_filesystem(file_system)
		    , m_name(name)
		{
			set_inode_number(inode_number);
		}

		Ext2File(Ext2FileSystem *file_system, uint32_t inode_number, char *name, FileType type)
		    : m_filesystem(file_system)
		    , m_name(name)
			, m_type(type)
		{
			set_inode_number(inode_number);
		}

		Ext2File(Ext2FileSystem *file_system, uint32_t inode_number, char *name, Ext2::inode_t metadata)
		    : m_filesystem(file_system)
		    , m_inode_metadata_cached(true)
		    , m_inode_metadata(metadata)
		    , m_size((uint64_t)metadata.size_low | ((uint64_t)metadata.size_high << 32))
		    , m_name(name)
			, m_type(from_inode_type(metadata.permission_type.type))
		{
			set_inode_number(inode_number);
		}

		// Basic file operations
		size_t read(size_t offset, size_t bytes, char *buffer) override;
		size_t write(size_t offset, size_t bytes, char *buffer) override;
		bool remove() override;
		bool rename(const LibK::string &new_file_name) override;
		bool is_type(FileType type) override { return m_type == type; };

		// Directory operations
		LibK::vector<File *> read_directory() override;
		File *find_file(const LibK::string &file_name) override;
		File *make_file(const LibK::string &file_name) override;

		LibK::StringView name() override { return LibK::StringView(m_name); };

		[[nodiscard]] size_t size() override;

	private:
		static FileType from_inode_type(Ext2::InodeType type);

		void read_and_parse_metadata();

		[[nodiscard]] bool can_open_for_read() const override { return true; };
		[[nodiscard]] bool can_open_for_write() const override { return open_write_contexts() == 0; };

		Ext2FileSystem *m_filesystem{nullptr};
		bool m_inode_metadata_cached{false};
		Ext2::inode_t m_inode_metadata{};
		uint64_t m_size{0};
		LibK::string m_name{};

		Locking::Mutex m_lock{};

		FileType m_type{};
	};

	class Ext2FileSystem final : public FileSystem
	{
		friend Ext2File;

		typedef struct __superblock_t
		{
			uint32_t inode_count;
			uint32_t block_count;
			uint32_t reserved_blocks;
			uint32_t unallocated_blocks;
			uint32_t unallocated_inodes;
			uint32_t superblock_block;
			int32_t block_size;
			int32_t fragment_size;
			uint32_t blocks_per_block_group;
			uint32_t fragments_per_block_group;
			uint32_t inodes_per_block_group;
			uint32_t last_mount_time;
			uint32_t last_written_time;
			uint16_t mount_count_since_fsck;
			uint16_t max_count_before_fsck;
			uint16_t signature;
			uint16_t state;
			uint16_t error_action;
			uint16_t minor;
			uint32_t time_of_last_fsck;
			uint32_t interval_between_fsck;
			uint32_t os_id;
			uint32_t major;
			uint16_t reserved_uid;
			uint16_t reserved_gid;

			// extended superblock fields
			uint32_t first_usable_inode;
			uint16_t inode_size;
			uint16_t superblock_block_group;

			// optional features
			uint32_t preallocate_blocks : 1;
			uint32_t afs_server_inodes : 1;
			uint32_t has_journal : 1;
			uint32_t inode_ext_attributes : 1;
			uint32_t can_resize : 1;
			uint32_t directory_hash_index : 1, : 26;

			// required features
			uint32_t uses_compression : 1;
			uint32_t directory_types : 1;
			uint32_t needs_journal_replay : 1;
			uint32_t uses_journal_device : 1, : 28;

			// read-only features
			uint32_t sparse_superblocks_and_group_descriptor_tables : 1;
			uint32_t long_file_size : 1;
			uint32_t binary_tree_directories : 1, : 29;

			uint8_t filesystem_id[16];
			char name[16];
			char last_mount_path[64];
			uint32_t compression_algorithms;
			uint8_t file_preallocated_blocks;
			uint8_t directory_preallocated_blocks;
			uint16_t rsv1;
			uint8_t journal_id[16];
			uint32_t journal_inode;
			uint32_t journal_device;
			uint32_t orphan_inode_list_head;
			uint8_t rsv2[1024 - 236];
		} __packed superblock_t;

		typedef struct __block_group_descriptor_t
		{
			uint32_t block_usage_bitmap_block;
			uint32_t inode_usage_bitmap_block;
			uint32_t inode_table_start_block;
			uint16_t unallocated_blocks;
			uint16_t unallocated_inodes;
			uint16_t directory_count;
			uint8_t rsv1[32 - 18];
		} __packed block_group_descriptor_t;

		enum class DirectoryEntryType : uint8_t
		{
			Unknown = 0,
			RegularFile = 1,
			Directory = 2,
			CharDevice = 3,
			BlockDevice = 4,
			FIFO = 5,
			Socket = 6,
			SoftLink = 7,
		};

		typedef struct __directory_entry_t
		{
			uint32_t inode;
			uint16_t size;
			uint8_t name_length_low;

			union
			{
				uint8_t name_length_high;
				DirectoryEntryType type_indicator;
			};

			char name[];
		} __packed directory_entry_t;

	public:
		explicit Ext2FileSystem(BlockDevice &device)
		    : m_device(&device)
		{
		}

		bool mount() override;
		bool unmount() override;

		// Basic file operations
		size_t read(size_t, size_t, char *) override { return 0; };
		size_t write(size_t, size_t, char *) override { return 0; };
		bool remove() override { return false; };
		bool rename(const LibK::string &) override { return false; };
		bool is_type(FileType type) override { return type == FileType::Directory; };

		// Directory operations
		LibK::vector<File *> read_directory() override;
		File *find_file(const LibK::string &file_name) override;
		File *make_file(const LibK::string &file_name) override;

		LibK::StringView name() override { return {}; };

		size_t read_blocks(size_t block, size_t count, char *buffer);
		size_t write_blocks(size_t block, size_t count, char *buffer);

		fs_block_t *read_block(size_t block);

		block_group_descriptor_t *get_block_group_descriptor(size_t index);
		void sync_block_group_descriptor(size_t index);

		LibK::vector<size_t> allocate_blocks(size_t count);

		[[nodiscard]] size_t size() override { return 0; }

	private:
		Ext2::inode_t read_inode_metadata(uint32_t inode_number);
		void write_inode_metadata(uint32_t inode_number, const Ext2::inode_t &inode);
		LibK::vector<File *> read_directory(const Ext2::inode_t &inode);

		static FileType from_directory_entry_type(DirectoryEntryType type);

		[[nodiscard]] bool can_open_for_read() const override { return true; };
		[[nodiscard]] bool can_open_for_write() const override { return true; };

		BlockDevice *m_device{nullptr};
		FileContext m_file{};

		superblock_t *m_superblock{nullptr};
		Locking::Mutex m_superblock_lock{};
		fs_block_t *m_superblock_block{nullptr};
		LibK::vector<fs_block_t *> m_bgd_table_blocks{};
		LibK::vector<Locking::Mutex *> m_block_group_locks{};

		uint32_t m_sectors_per_block{};
		uint32_t m_block_size{};
		uint32_t m_number_of_block_groups{};

		Ext2::inode_t root_inode{};
	};
}
