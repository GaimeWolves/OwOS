#pragma once

#include <devices/BlockDevice.hpp>
#include <locking/Mutex.hpp>

namespace Kernel
{
	typedef struct __fs_block_t
	{
		BlockDevice *device{nullptr};
		size_t block{0};
		Memory::memory_region_t region{};
		Locking::Mutex lock{};
		size_t refcount{0};

		bool operator==(const __fs_block_t &other) const { return this->device == other.device && this->block == other.block; }
		bool operator<(const __fs_block_t &other) const { return this->device < other.device || (this->device == other.device && this->block < other.block); }

		[[nodiscard]] char *data() const { return (char *)region.virt_region().pointer(); };
	} fs_block_t;

	class FileSystemCache
	{
	public:
		static fs_block_t *acquire(BlockDevice *device, size_t block);
		static void sync(fs_block_t *block);
		static void release(fs_block_t *block);
	};
}
