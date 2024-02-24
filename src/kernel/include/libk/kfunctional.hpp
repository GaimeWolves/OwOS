#pragma once

#include <libk/kcassert.hpp>
#include <libk/kcstddef.hpp>
#include <libk/kcstring.hpp>
#include <libk/kutility.hpp>
#include <libk/kshared_ptr.hpp>

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
			return (*functor)(std::forward<Args>(args)...);
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
		{
		}

		template <typename Functor>
		function(Functor functor)
		    : m_invoke_fn(reinterpret_cast<invoke_fn_t>(invoke_fn<Functor>))
		    , m_construct_fn(reinterpret_cast<construct_fn_t>(construct_fn<Functor>))
		    , m_destroy_fn(reinterpret_cast<destroy_fn_t>(destroy_fn<Functor>))
		    , m_functor_size(sizeof(Functor))
		{
			if (m_functor_size > SMALL_SIZE)
			{
				m_functor_ptr = shared_ptr<char>(new char[m_functor_size]);
				m_construct_fn(m_functor_ptr.get(), reinterpret_cast<char *>(&functor));
			}
			else
				m_construct_fn(m_functor_small, reinterpret_cast<char *>(&functor));
		}

		function(function const &rhs)
		    : m_invoke_fn(rhs.m_invoke_fn)
		    , m_construct_fn(rhs.m_construct_fn)
		    , m_destroy_fn(rhs.m_destroy_fn)
		    , m_functor_ptr(rhs.m_functor_ptr)
		    , m_functor_size(rhs.m_functor_size)
		{
			if (m_functor_size <= SMALL_SIZE)
				m_construct_fn(m_functor_small, (char *)rhs.m_functor_small);
		}

		template <typename Functor>
		function &operator=(Functor functor)
		{
			this->~function();

			m_invoke_fn = reinterpret_cast<invoke_fn_t>(invoke_fn<Functor>);
			m_construct_fn = reinterpret_cast<construct_fn_t>(construct_fn<Functor>);
			m_destroy_fn = reinterpret_cast<destroy_fn_t>(destroy_fn<Functor>);
			m_functor_size = sizeof(Functor);

			if (m_functor_size > SMALL_SIZE)
			{
				m_functor_ptr = shared_ptr<char>(new char[m_functor_size]);
				m_construct_fn(m_functor_ptr.get(), reinterpret_cast<char *>(&functor));
			}
			else
				m_construct_fn(m_functor_small, reinterpret_cast<char *>(&functor));

			return *this;
		}

		function &operator=(const function &rhs)
		{
			this->~function();

			m_invoke_fn = rhs.m_invoke_fn;
			m_construct_fn = rhs.m_construct_fn;
			m_functor_size = rhs.m_functor_size;
			m_destroy_fn = rhs.m_destroy_fn;
			m_functor_ptr = rhs.m_functor_ptr;

			if (m_invoke_fn && m_functor_size <= SMALL_SIZE)
				m_construct_fn(m_functor_small, (char *)rhs.m_functor_small);

			return *this;
		}

		function &operator=(function &&rhs) noexcept
		{
			this->~function();

			m_invoke_fn = rhs.m_invoke_fn;
			m_construct_fn = rhs.m_construct_fn;
			m_functor_size = rhs.m_functor_size;
			m_destroy_fn = rhs.m_destroy_fn;
			m_functor_ptr = rhs.m_functor_ptr;

			if (m_invoke_fn && m_functor_size <= SMALL_SIZE)
				m_construct_fn(m_functor_small, (char *)rhs.m_functor_small);

			rhs.~function();

			return *this;
		}

		~function()
		{
			if (m_destroy_fn)
			{
				if (m_functor_ptr)
					m_destroy_fn(m_functor_ptr.get());
				else if (m_functor_size > 0)
					m_destroy_fn(m_functor_small);
			}

			// TODO: Investigate why this breaks stuff
			//m_invoke_fn = nullptr;
			//m_construct_fn = nullptr;
			//m_destroy_fn = nullptr;
			m_functor_size = 0;
			m_functor_ptr = shared_ptr<char>(nullptr);
		}

		Ret operator()(Args...args) const
		{
			if (m_functor_ptr)
				return m_invoke_fn(m_functor_ptr.get(), std::forward<Args>(args)...);
			else
				return m_invoke_fn((char *)m_functor_small, std::forward<Args>(args)...);
		}

		explicit operator bool() const noexcept { return m_invoke_fn; }

	private:
		static constexpr size_t SMALL_SIZE = 32;

		invoke_fn_t m_invoke_fn{nullptr};
		construct_fn_t m_construct_fn{nullptr};
		destroy_fn_t m_destroy_fn{nullptr};

		// TODO: Use an unique_ptr here
		shared_ptr<char> m_functor_ptr{nullptr};
		size_t m_functor_size{0};

		char m_functor_small[SMALL_SIZE]{};
	};
} // namespace Kernel::LibK
