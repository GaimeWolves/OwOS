#pragma once

#include <processes/definitions.hpp>

namespace Kernel
{
	class CoreScheduler
	{
	public:
		static void initialize();

		static void suspend(thread_t *thread);
		static void resume(thread_t *thread);

		static void block(Locking::Mutex *lock);
		static void terminate(thread_t *thread);
		static void terminate_current();

		static constexpr uint64_t SMALLEST_INTERVAL = 10 * 1000 * 1000;
	private:
		static void tick();
		__noreturn static void idle();

		static thread_t *pick_next();
	};
}