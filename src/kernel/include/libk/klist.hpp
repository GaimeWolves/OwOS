#pragma once

namespace Kernel::LibK
{
	template <class T>
	class list
	{
		typedef struct __list_node_t
		{
			T data;
			__list_node_t *next;
			__list_node_t *prev;
		} list_node_t;

	public:
		explicit list()
		{
			head.next = &head;
			head.prev = &head;
		}

		T &front() { return head.next->data; }

		T &back() { return head.prev->data; }

		void push_back(T &&data)
		{
			auto *node = new list_node_t;

			node->data = LibK::move(data);

			node->prev = head.prev;
			head.prev->next = node;
			node->next = &head;
			head.prev = node;
		}

		void push_back(const T &data) { push_back((T &&)data); }

		void push_front(T &&data)
		{
			auto *node = new list_node_t;

			node->data = LibK::move(data);

			node->next = head.next;
			head.next->prev = node;
			node->prev = &head;
			head.next = node;
		}

		void push_front(const T &data) { push_front(move(data)); }

		T pop_front()
		{
			assert(!empty());

			auto *node = head.next;
			node->next->prev = &head;
			head.next = node->next;

			T data = LibK::move(node->data);
			delete node;

			return LibK::move(data);
		}

		T pop_back()
		{
			assert(!empty());

			auto *node = head.prev;
			node->prev->next = &head;
			head.prev = node->prev;

			T data = LibK::move(node->data);
			delete node;

			return LibK::move(data);
		}

		[[nodiscard]] bool empty() const { return head.next == &head; }

	private:
		list_node_t head{};
	};
}
