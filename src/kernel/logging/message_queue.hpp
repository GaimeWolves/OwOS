#pragma once

#include "arch/spinlock.hpp"
#include "libk/StringView.hpp"
#include "libk/katomic.hpp"
#include "libk/kstring.hpp"

namespace Kernel
{
	class MessageQueue
	{
		typedef struct __list_node_t
		{
			LibK::string message; // TODO: A string may not be the best data type due to heap allocations
			__list_node_t *next;
			__list_node_t *prev;
		} list_node_t;

	public:
		explicit MessageQueue()
		{
			head.next = &head;
			head.prev = &head;
		}

		LibK::string get()
		{
			assert(!empty());

			auto *node = head.next;

			queue_lock.lock();
			head.next = node->next;
			head.next->prev = &head;
			queue_lock.unlock();

			LibK::string message = LibK::move(node->message);
			kfree(node);

			return LibK::move(message);
		}

		void put(LibK::string &&message)
		{
			auto *node = static_cast<list_node_t *>(kmalloc(sizeof(list_node_t)));

			node->message = LibK::move(message);

			queue_lock.lock();
			node->prev = head.prev;
			head.prev->next = node;
			node->next = &head;
			head.prev = node;
			queue_lock.unlock();
		}

		[[nodiscard]] bool empty() const { return head.next == &head; }

	private:
		list_node_t head{};

		// TODO: Think more about a lockless design.
		//       For now it locks the queue in the very short intervals of adjusting the heads next and prev pointers.
		Locking::Spinlock queue_lock;
	};
}