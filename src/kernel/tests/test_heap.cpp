#include <tests.hpp>

#include "definitions.hpp"

#include <libk/kcmalloc.hpp>
#include <logging/logger.hpp>

namespace Kernel::Tests
{
	using Kernel::Heap::getStatistics;
	using Kernel::Heap::heap_statistics_t;

	static bool test_simple_kmalloc();
	static bool test_large_kmalloc();
	static bool test_many_kmalloc();

	static heap_statistics_t orig_stats;

	static bool test_simple_kmalloc()
	{
		orig_stats = getStatistics();

		auto ptr = kmalloc(0x10);

		if (ptr == nullptr)
		{
			log(get_tag(false), "Simple allocation: nullptr returned");
			return false;
		}

		kfree(ptr);

		auto stats = getStatistics();

		if (stats.used != orig_stats.used || stats.free != orig_stats.free)
		{
			log(get_tag(false), "Simple allocation: statistics wrong (heap corrupted?)");
			return false;
		}

		log(get_tag(true), "Simple allocation");
		return true;
	}

	static bool test_large_kmalloc()
	{
		orig_stats = getStatistics();

		auto ptr = kmalloc(0x1000);

		if (ptr == nullptr)
		{
			log(get_tag(false), "Large allocation: nullptr returned");
			return false;
		}

		kfree(ptr);

		auto stats = getStatistics();

		if (stats.used != orig_stats.used || stats.free != orig_stats.free)
		{
			log(get_tag(false), "Large allocation: statistics wrong (heap corrupted?)");
			return false;
		}

		log(get_tag(true), "Large allocation");
		return true;
	}

	static bool test_many_kmalloc()
	{
		void *allocs[100] = {0};

		orig_stats = getStatistics();

		for (uint32_t i = 0; i < sizeof(allocs) / sizeof(void *); i++)
			allocs[i] = kmalloc(7 * (i + 1));

		for (auto ptr : allocs)
		{
			if (ptr == nullptr)
			{
				log(get_tag(false), "Many allocations: nullptr returned");
				return false;
			}

			kfree(ptr);
		}

		auto stats = getStatistics();

		if (stats.used != orig_stats.used || stats.free != orig_stats.free)
		{
			log(get_tag(false), "Many allocations: statistics wrong (heap corrupted?)");
			return false;
		}

		log(get_tag(true), "Many allocations");
		return true;
	}

	bool test_heap()
	{
		log("TEST", "Heap allocations");
		bool ok = true;

		if (!test_simple_kmalloc())
			ok = false;

		if (!test_large_kmalloc())
			ok = false;

		if (!test_many_kmalloc())
			ok = false;

		return ok;
	}

} // namespace Kernel::Tests