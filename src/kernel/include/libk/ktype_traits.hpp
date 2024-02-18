#pragma once

namespace Kernel::LibK
{
	template <class T, T v>
	struct integral_constant
	{
		static constexpr T value = v;

		typedef T value_type;
		typedef integral_constant<T, v> type;

		constexpr explicit operator T() const noexcept { return v; }
		constexpr T operator()() const noexcept { return v; }
	};

	typedef integral_constant<bool, true> true_type;
	typedef integral_constant<bool, false> false_type;

	template <bool Cond, class T = void>
	struct enable_if
	{
	};

	template <class T>
	struct enable_if<true, T>
	{
		typedef T type;
	};

	template <class T>
	struct remove_const
	{
		typedef T type;
	};

	template <class T>
	struct remove_const<const T>
	{
		typedef T type;
	};

	template <class T>
	struct remove_volatile
	{
		typedef T type;
	};

	template <class T>
	struct remove_volatile<volatile T>
	{
		typedef T type;
	};

	template <class T>
	struct remove_cv
	{
		typedef typename remove_const<typename remove_volatile<T>::type>::type type;
	};

	template <class T>
	struct remove_reference
	{
		typedef T type;
	};

	template <class T>
	struct remove_reference<T &>
	{
		typedef T type;
	};

	template <class T>
	struct remove_reference<T &&>
	{
		typedef T type;
	};

	template <class T>
	struct is_const : false_type
	{
	};

	template <class T>
	struct is_const<const T> : true_type
	{
	};

	template <class T>
	struct is_lvalue_reference : false_type
	{
	};

	template <class T>
	struct is_lvalue_reference<T &> : true_type
	{
	};

	template <class T>
	struct is_rvalue_reference : false_type
	{
	};

	template <class T>
	struct is_rvalue_reference<T &&> : true_type
	{
	};

	template <class T>
	struct is_reference : integral_constant<bool, is_rvalue_reference<T>::value || is_lvalue_reference<T>::value>
	{
	};

	template <class T>
	struct is_function : integral_constant<bool, !is_const<const T>::value && !is_reference<T>::value>
	{
	};

	template <class T>
	struct __is_pointer_helper : false_type
	{
	};

	template <class T>
	struct __is_pointer_helper<T *> : true_type
	{
	};

	template <class T>
	struct is_pointer : __is_pointer_helper<typename remove_cv<T>::type>
	{
	};

	template <class T>
	struct remove_pointer
	{
		typedef T type;
	};

	template <class T>
	struct remove_pointer<T *>
	{
		typedef T type;
	};

	template <class T>
	struct remove_pointer<T *const>
	{
		typedef T type;
	};

	template <class T>
	struct remove_pointer<T *volatile>
	{
		typedef T type;
	};

	template <class T>
	struct remove_pointer<T *const volatile>
	{
		typedef T type;
	};

} // namespace Kernel::LibK
