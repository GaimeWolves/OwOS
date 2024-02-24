#pragma once

#include <arch/spinlock.hpp>
#include <libk/StringView.hpp>
#include <libk/kstring.hpp>

namespace Kernel::LibK
{
	template <typename T>
	class SRMWQueue
	{
		typedef struct __list_node_t
		{
			T data;
			__list_node_t *next;
			__list_node_t *prev;
		} list_node_t;

	public:
		explicit SRMWQueue()
		{
			head.next = &head;
			head.prev = &head;
		}

		T get()
		{
			assert(!empty());

			queue_lock.lock();
			auto *node = head.next;
			node->next->prev = &head;
			head.next = node->next;
			queue_lock.unlock();

			T data = std::move(node->data);
			delete node;

			return std::move(data);
		}

		void put(T &&data)
		{
			auto *node = new list_node_t;

			node->data = std::move(data);

			queue_lock.lock();
			node->prev = head.prev;
			head.prev->next = node;
			node->next = &head;
			head.prev = node;
			queue_lock.unlock();
		}

		void put(const T &data)
		{
			put(move(data));
		}

		[[nodiscard]] bool empty() const { return head.next == &head; }

	private:
		list_node_t head{};

		// TODO: Think more about a lockless design.
		//       For now it locks the queue in the very short intervals of adjusting the heads next and prev pointers.
		Locking::Spinlock queue_lock;
	};
}
