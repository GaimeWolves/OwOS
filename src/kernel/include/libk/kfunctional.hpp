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
	public:
		function() = default;

		function(LibK::nullptr_t)
		    : m_callable(nullptr)
		{
		}

		template <typename F>
		function(const function &other)
		{
			if (other.m_callable)
				m_callable = new callable<F>(*other.m_callable);
			else
				m_callable = nullptr;
		}

		template <typename F>
		function(F functor)
		    : m_callable(new callable<F>(move(functor)))
		{
		}

		template <typename F>
		function &operator=(F functor)
		{
			m_callable = new callable<F>(move(functor));
			return *this;
		}

		template <typename F>
		function &operator=(const function &other)
		{
			if (other.m_callable)
				m_callable = new callable<F>(*other.m_callable);
			else
				m_callable = nullptr;

			return *this;
		}

		function &operator=(LibK::nullptr_t)
		{
			m_callable = nullptr;
			return *this;
		}

		~function()
		{
			delete m_callable;
		}

		Ret operator()(Args... args) const
		{
			assert(m_callable);
			return m_callable->call(forward<Args>(args)...);
		}

		explicit operator bool() const noexcept { return m_callable != nullptr; }

	private:
		struct callable_base
		{
		public:
			virtual ~callable_base() = default;
			virtual Ret call(Args...) const = 0;
		};

		template <typename F>
		struct callable : callable_base
		{
			F functor;

			callable(F &&functor)
			    : functor(move(functor))
			{
			}

			callable(const callable &other)
			    : functor(other.functor)
			{
			}

			Ret call(Args... args) const final override { return functor(forward<Args>(args)...); }
		};

		// TODO: Implement unique_ptr
		callable_base *m_callable{nullptr};
	};
} // namespace Kernel::LibK
