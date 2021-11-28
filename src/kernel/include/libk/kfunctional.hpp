#pragma once

#include <libk/kcassert.hpp>
#include <libk/kcstddef.hpp>
#include <libk/ktype_traits.hpp>
#include <libk/kutility.hpp>

namespace Kernel::LibK
{
	template <class T>
	class function;

	template <class Ret, class... Args>
	class function<Ret(Args...)>
	{
	private:
		typedef Ret (*invoke_fn_t)(char *, Args &&...);
		typedef void (*construct_fn_t)(char *, char *);
		typedef void (*destroy_fn_t)(char *);

		template <typename Functor>
		static Ret invoke_fn(Functor *functor, Args &&... args)
		{
			return (*functor)(LibK::forward<Args>(args)...);
		}

		template <typename Functor>
		static void construct_fn(Functor *construct_dst, Functor *construct_src)
		{
			new (construct_dst) Functor(*construct_src);
		}

		template <typename Functor>
		static void destroy_fn(Functor *f)
		{
			f->~Functor();
		}

	public:
		function()
		    : m_invoke_fn(nullptr), m_construct_fn(nullptr), m_destroy_fn(nullptr), m_functor_ptr(nullptr), m_functor_size(0)
		{
		}

		template <typename Functor>
		function(Functor functor)
		    : m_invoke_fn(reinterpret_cast<invoke_fn_t>(invoke_fn<Functor>))
		    , m_construct_fn(reinterpret_cast<construct_fn_t>(construct_fn<Functor>))
		    , m_destroy_fn(reinterpret_cast<destroy_fn_t>(destroy_fn<Functor>))
		    , m_functor_ptr(new char[sizeof(Functor)])
		    , m_functor_size(sizeof(Functor))
		{
			m_construct_fn(m_functor_ptr, reinterpret_cast<char *>(&functor));
		}

		function(function const &rhs)
		    : m_invoke_fn(rhs.m_invoke_fn)
		    , m_construct_fn(rhs.m_construct_fn)
		    , m_destroy_fn(rhs.m_destroy_fn)
		    , m_functor_ptr(nullptr)
		    , m_functor_size(rhs.m_functor_size)
		{
			if (m_invoke_fn)
			{
				rhs.m_destroy_fn(rhs.m_functor_ptr);
				m_functor_ptr = new char[m_functor_size];
				m_construct_fn(m_functor_ptr, rhs.m_functor_ptr);
			}
		}

		template <typename Functor>
		function &operator=(Functor functor)
		{
			if (m_functor_ptr)
			{
				m_destroy_fn(m_functor_ptr);
			}

			m_invoke_fn = reinterpret_cast<invoke_fn_t>(invoke_fn<Functor>);
			m_construct_fn = reinterpret_cast<construct_fn_t>(construct_fn<Functor>);
			m_destroy_fn = reinterpret_cast<destroy_fn_t>(destroy_fn<Functor>);
			m_functor_ptr = new char[sizeof(Functor)];
			m_functor_size = sizeof(Functor);

			m_construct_fn(m_functor_ptr, reinterpret_cast<char *>(&functor));

			return *this;
		}

		function &operator=(const function &rhs)
		{
			m_invoke_fn = rhs.m_invoke_fn;
			m_construct_fn = rhs.m_construct_fn;
			m_destroy_fn = rhs.m_destroy_fn;
			m_functor_size = rhs.m_functor_size;

			if (m_invoke_fn)
			{
				if (m_functor_ptr)
				{
					m_destroy_fn(m_functor_ptr);
				}

				m_functor_ptr = new char[m_functor_size];
				m_construct_fn(m_functor_ptr, rhs.m_functor_ptr);
			}

			return *this;
		}

		~function()
		{
			if (m_functor_ptr)
			{
				m_destroy_fn(m_functor_ptr);
			}
		}

		Ret operator()(Args... args)
		{
			return m_invoke_fn(m_functor_ptr, LibK::forward<Args>(args)...);
		}

		explicit operator bool() const noexcept { return m_functor_ptr != nullptr; }

	private:
		invoke_fn_t m_invoke_fn;
		construct_fn_t m_construct_fn;
		destroy_fn_t m_destroy_fn;

		// TODO: Use an unique_ptr here
		char *m_functor_ptr;
		size_t m_functor_size;
	};
} // namespace Kernel::LibK
