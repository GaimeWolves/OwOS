#pragma once

#include <stddef.h>

#include <libk/StringView.hpp>
#include <libk/kstring.hpp>
#include <libk/kvector.hpp>
#include <libk/ErrorOr.hpp>

#include <locking/Mutex.hpp>
#include <filesystem/definitions.hpp>
#include <memory/VirtualMemoryManager.hpp>

// These flags match one to one with the POSIX fcntl.h definitions
// See: https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/fcntl.h.html
// Todo: This needs to move to the libc includes (or be imported from there)
#define O_EXEC       0x00000001
#define O_RDONLY     0x00000002
#define O_RDWR       0x00000004
#define O_SEARCH     0x00000008
#define O_WRONLY     0x00000010
#define O_APPEND     0x00000020
#define O_CLOEXEC    0x00000040
#define O_CREAT      0x00000080
#define O_DIRECTORY  0x00000100
#define O_DSYNC      0x00000200
#define O_EXCL       0x00000400
#define O_NOCTTY     0x00000800
#define O_NOFOLLOW   0x00001000
#define O_NONBLOCK   0x00002000
#define O_RSYNC      0x00004000
#define O_SYNC       0x00008000
#define O_TRUNC      0x00010000
#define O_TTY_INIT   0x00020000

namespace Kernel
{
	enum class FileType
	{
		Unknown,
		RegularFile,
		Directory,
		Device,
		SoftLink,
	};

	class File
	{
		friend class VirtualFileSystem;

	public:
		// Basic file operations
		virtual FileContext open(int options);
		virtual bool close(FileContext &context);
		virtual size_t read(size_t offset, size_t bytes, char *buffer) = 0;
		virtual size_t write(size_t offset, size_t bytes, char *buffer) = 0;
		virtual bool remove() = 0;
		virtual bool rename(const LibK::string &new_file_name) = 0;
		virtual bool is_type(FileType type) = 0;

		// Directory operations
		virtual LibK::vector<File *> read_directory() = 0;
		virtual File *find_file(const LibK::string &file_name) = 0;
		virtual File *make_file(const LibK::string &file_name) = 0;

		virtual LibK::ErrorOr<void> ioctl(uint32_t, uintptr_t) { return ENOTTY; };

		virtual LibK::StringView name() = 0;

		[[nodiscard]] virtual size_t size() = 0;

		void lock() { m_mutex->lock(); }
		void unlock() { m_mutex->unlock(); }

		[[nodiscard]] size_t inode_number() const { return m_inode_number; }

	protected:
		File()
		    : m_mutex(new Locking::Mutex())
		{
		}

		~File()
		{
			delete m_mutex;
		}

		[[nodiscard]] virtual bool can_open_for_read() const = 0;
		[[nodiscard]] virtual bool can_open_for_write() const = 0;

		[[nodiscard]] size_t open_read_contexts() const { return m_open_read_contexts; }
		[[nodiscard]] size_t open_write_contexts() const { return m_open_write_contexts; }

		void set_inode_number(size_t inode_number) { m_inode_number = inode_number; }

	private:
		void set_vfs_node(vfs_node_t *node) { m_vfs_node = node; }

		vfs_node_t *m_vfs_node{nullptr};

		size_t m_open_read_contexts{0};
		size_t m_open_write_contexts{0};

		size_t m_inode_number{0};

		Locking::Mutex *m_mutex{};
	};
} // namespace Kernel