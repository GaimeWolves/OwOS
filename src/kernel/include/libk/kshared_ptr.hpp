#pragma once

#include <libk/kcstddef.hpp>
#include <libk/katomic.hpp>
#include <libk/kcassert.hpp>

namespace Kernel::LibK
{
	template <typename T>
	class shared_ptr
	{
		template <typename U>
		friend class shared_ptr;
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

			void destroy() override { deleter(pointer); }
		};

		template<class U>
		struct default_deleter
		{
			void operator()(U *ptr) const { delete ptr; }
		};

	public:
		constexpr shared_ptr() noexcept = default;
		constexpr explicit shared_ptr(std::nullptr_t) noexcept {};

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

		shared_ptr(const shared_ptr &rhs)
		    : m_aux(rhs.m_aux)
		    , m_data(rhs.m_data)
		{ increment(); }

		template<class U>
		shared_ptr(const shared_ptr<U> &rhs)
		    : m_aux(reinterpret_cast<shared_ptr<T>::auxiliary *>(rhs.m_aux))
		    , m_data(reinterpret_cast<T *>(rhs.m_data))
		{ increment(); }

		~shared_ptr()
		{
			decrement();
			m_aux = nullptr;
			m_data = nullptr;
		}

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
				m_aux->count.fetch_add(1);
		}

		void decrement()
		{
			if (m_aux)
			{
				size_t old_value = m_aux->count.fetch_sub(1);
				assert(old_value != 0);
				if (old_value == 1)
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
