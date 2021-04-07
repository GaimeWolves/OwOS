#include <tests.hpp>

#include <libk/kcmalloc.hpp>
#include <libk/kcstdio.hpp>

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
			LibK::printf_check_msg(false, "Simple allocation: nullptr returned");
			return false;
		}

		kfree(ptr);

		auto stats = getStatistics();

		if (stats.used != orig_stats.used || stats.free != orig_stats.free)
		{
			LibK::printf_check_msg(false, "Simple allocation: statistics wrong (heap corrupted?)");
			return false;
		}

		LibK::printf_check_msg(true, "Simple allocation");
		return true;
	}

	static bool test_large_kmalloc()
	{
		orig_stats = getStatistics();

		auto ptr = kmalloc(0x1000);

		if (ptr == nullptr)
		{
			LibK::printf_check_msg(false, "Large allocation: nullptr returned");
			return false;
		}

		kfree(ptr);

		auto stats = getStatistics();

		if (stats.used != orig_stats.used || stats.free != orig_stats.free)
		{
			LibK::printf_check_msg(false, "Large allocation: statistics wrong (heap corrupted?)");
			return false;
		}

		LibK::printf_check_msg(true, "Large allocation");
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
				LibK::printf_check_msg(false, "Many allocations: nullptr returned");
				return false;
			}

			kfree(ptr);
		}

		auto stats = getStatistics();

		if (stats.used != orig_stats.used || stats.free != orig_stats.free)
		{
			LibK::printf_check_msg(false, "Many allocations: statistics wrong (heap corrupted?)");
			return false;
		}

		LibK::printf_check_msg(true, "Many allocations");
		return true;
	}

	bool test_heap()
	{
		LibK::printf_test_msg("Heap allocations");
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