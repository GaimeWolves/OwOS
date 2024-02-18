#include <filesystem/FileSystemCache.hpp>

#include <libk/AVLTree.hpp>

namespace Kernel
{
	static Locking::Mutex s_fs_cache_lock;
	static LibK::AVLTree<fs_block_t> s_fs_cache;

	fs_block_t *FileSystemCache::acquire(BlockDevice *device, size_t block)
	{
		assert(device->block_size() <= PAGE_SIZE && PAGE_SIZE % device->block_size() == 0);

		fs_block_t *fs_block;

		s_fs_cache_lock.lock();
		fs_block = s_fs_cache.find({device, block});

		if (fs_block)
		{
			fs_block->lock.lock();
			fs_block->refcount++;
			fs_block->lock.unlock();
			return fs_block;
		}
		s_fs_cache_lock.unlock();

		s_fs_cache_lock.lock();
		fs_block = s_fs_cache.emplace(device, block);
		fs_block->refcount = 1;
		s_fs_cache_lock.unlock();
		fs_block->region = Memory::VirtualMemoryManager::instance().allocate_region(PAGE_SIZE);

		size_t blocks_per_cache_block = PAGE_SIZE / device->block_size();
		size_t read = fs_block->device->read_blocks(block * blocks_per_cache_block, blocks_per_cache_block, reinterpret_cast<char *>(fs_block->region.virt_address));
		assert(read > 0);

		return fs_block;
	}

	void FileSystemCache::sync(fs_block_t *block)
	{
		size_t blocks_per_cache_block = PAGE_SIZE / block->device->block_size();
		size_t written = block->device->write_blocks(block->block * blocks_per_cache_block, blocks_per_cache_block, reinterpret_cast<char *>(block->region.virt_address));
		assert(written > 0);
	}

	void FileSystemCache::release(fs_block_t *block)
	{
		s_fs_cache_lock.lock();
		block->lock.lock();
		block->refcount--;

		if (block->refcount == 0)
		{
			sync(block);
			Memory::VirtualMemoryManager::instance().free(block->region);
			bool removed = s_fs_cache.remove(*block);
			assert(removed);
		}
		else
		{
			block->lock.unlock();
		}
		s_fs_cache_lock.unlock();
	}
}
