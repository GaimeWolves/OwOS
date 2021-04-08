#pragma once

#include <libk/kcassert.hpp>
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
		template <typename F>
		function(F functor)
		    : m_callable(new callable<F>(move(functor)))
		{
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

			Ret call(Args... args) const final override { return functor(forward<Args>(args)...); }
		};

		// TODO: Implement unique_ptr
		callable_base *m_callable;
	};
} // namespace Kernel::LibK
