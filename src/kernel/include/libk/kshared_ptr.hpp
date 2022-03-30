#pragma once

#include <libk/katomic.hpp>

namespace Kernel::LibK
{
	template <typename T>
	class shared_ptr
	{
	private:
		struct auxiliary
		{
			atomic_size_t count;

			auxiliary()
				: count(1)
			{}

			virtual void destroy() = 0;
			virtual ~auxiliary() = default;
		};

		template <class U, class Deleter>
		struct auxiliary_impl : public auxiliary
		{
			U *pointer;
			Deleter deleter;

			auxiliary_impl(U *ptr, Deleter d)
				: pointer(ptr), deleter(d)
			{}

			virtual void destroy() { deleter(pointer); }
		};

		template<class U>
		struct default_deleter
		{
			void operator()(U *ptr) const { delete ptr; }
		};

	public:
		constexpr shared_ptr() noexcept = default;
		constexpr explicit shared_ptr(nullptr_t) noexcept {};

		template<class U, class Deleter>
		shared_ptr(U *ptr, Deleter d)
		    : m_aux(new auxiliary_impl<U, Deleter>(ptr, d))
		    , m_data(ptr)
		{}

		template<class U>
		explicit shared_ptr(U *ptr)
		    : m_aux(new auxiliary_impl<U, default_deleter<U>>(ptr, default_deleter<U>()))
		    , m_data(ptr)
		{}

		shared_ptr(const shared_ptr& rhs)
		    : m_aux(rhs.m_aux)
		    , m_data(rhs.m_data)
		{ increment(); }

		template<class U>
		shared_ptr(const shared_ptr<U> &rhs)
		    : m_aux(rhs.m_aux)
		    , m_data(rhs.m_data)
		{ increment(); }

		~shared_ptr() { decrement(); }

		shared_ptr &operator=(const shared_ptr &rhs)
		{
			if(this != &rhs)
			{
				decrement();

				m_aux = rhs.m_aux;
				m_data = rhs.m_data;

				increment();
			}

			return *this;
		}

		shared_ptr &operator=(shared_ptr &&rhs)
		{
			if(this != &rhs)
			{
				decrement();

				m_aux = rhs.m_aux;
				m_data = rhs.m_data;

				rhs.m_aux = nullptr;
				rhs.m_data = nullptr;
			}

			return *this;
		}

		T *get() const { return m_data; }
		T *operator->() const { return m_data; }
		T &operator*() const { return *m_data; }

		explicit operator bool() const noexcept { return m_data != nullptr; }

	private:
		void increment()
		{
			if (m_aux)
				m_aux->count++;
		}

		void decrement()
		{
			if (m_aux)
			{
				m_aux->count--;

				if (m_aux->count == 0)
				{
					m_aux->destroy();
					delete m_aux;
				}
			}
		}

		auxiliary *m_aux{nullptr};
		T *m_data{nullptr};
	};
}