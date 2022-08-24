#pragma once

namespace LibK
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

		[[nodiscard]] bool empty() const { return head.next == &head; }

	private:
		list_node_t head{};
	};
}