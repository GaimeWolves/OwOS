#pragma once

#include <stddef.h>
#include <stdint.h>

#include <common_attributes.h>

namespace Kernel::LibK
{
	enum MemoryOrder
	{
		memory_order_relaxed = __ATOMIC_RELAXED, // no ordering constraints
		memory_order_consume = __ATOMIC_CONSUME, // same as MemoryOrder::Acquire
		memory_order_acquire = __ATOMIC_ACQUIRE, // prevents hoisting of code to before this operation
		memory_order_release = __ATOMIC_RELEASE, // prevents sinking of code to after this operation
		memory_order_acq_rel = __ATOMIC_ACQ_REL, // combines MemoryOrder::Acquire and MemoryOrder::Release
		memory_order_seq_cst = __ATOMIC_SEQ_CST, // enforces total sequential ordering
	};

	template <class T>
	struct atomic
	{
	private:
		T m_value{0};

	public:
		atomic() noexcept = default;
		atomic &operator=(const atomic &) volatile = delete;
		atomic &operator=(atomic &&) volatile = delete;
		atomic(const atomic &) = delete;
		atomic(atomic &&) = delete;

		constexpr atomic(T value) noexcept
		    : m_value(value)
		{
		}

		static constexpr bool is_always_lock_free = __atomic_always_lock_free(sizeof(T), 0);

		always_inline bool is_lock_free() const volatile noexcept
		{
			return __atomic_is_lock_free(sizeof m_value, &m_value);
		}

		always_inline void store(T desired, MemoryOrder order = memory_order_seq_cst) volatile noexcept
		{
			__atomic_store_n(&m_value, desired, order);
		}

		always_inline T load(MemoryOrder order = memory_order_seq_cst) const volatile noexcept
		{
			return __atomic_load_n(&m_value, order);
		}

		always_inline T exchange(T desired, MemoryOrder order = memory_order_seq_cst) volatile noexcept
		{
			return __atomic_exchange_n(&m_value, desired, order);
		}

		always_inline bool compare_exchange_weak(T &expected, T desired, MemoryOrder success, MemoryOrder failure) volatile noexcept
		{
			return __atomic_compare_exchange_n(&m_value, &expected, desired, true, success, failure);
		}

		always_inline bool compare_exchange_weak(T &expected, T desired, MemoryOrder order = memory_order_seq_cst) volatile noexcept
		{
			if (order == memory_order_acq_rel)
				return __atomic_compare_exchange_n(&m_value, &expected, desired, true, order, memory_order_acquire);

			if (order == memory_order_release)
				return __atomic_compare_exchange_n(&m_value, &expected, desired, true, order, memory_order_relaxed);

			return __atomic_compare_exchange_n(&m_value, &expected, desired, true, order, order);
		}

		always_inline bool compare_exchange_strong(T &expected, T desired, MemoryOrder success, MemoryOrder failure) volatile noexcept
		{
			return __atomic_compare_exchange_n(&m_value, &expected, desired, false, success, failure);
		}

		always_inline bool compare_exchange_strong(T &expected, T desired, MemoryOrder order = memory_order_seq_cst) volatile noexcept
		{
			if (order == memory_order_acq_rel)
				return __atomic_compare_exchange_n(&m_value, &expected, desired, false, order, memory_order_acquire);

			if (order == memory_order_release)
				return __atomic_compare_exchange_n(&m_value, &expected, desired, false, order, memory_order_relaxed);

			return __atomic_compare_exchange_n(&m_value, &expected, desired, false, order, order);
		}

		// TODO: Implement is_integral and is_floating in type_traits.hpp
		always_inline T fetch_add(T arg, MemoryOrder order = memory_order_seq_cst) volatile noexcept
		{
			return __atomic_fetch_add(&m_value, arg, order);
		}

		always_inline T fetch_sub(T arg, MemoryOrder order = memory_order_seq_cst) volatile noexcept
		{
			return __atomic_fetch_sub(&m_value, arg, order);
		}

		always_inline T fetch_and(T arg, MemoryOrder order = memory_order_seq_cst) volatile noexcept
		{
			return __atomic_fetch_and(&m_value, arg, order);
		}

		always_inline T fetch_or(T arg, MemoryOrder order = memory_order_seq_cst) volatile noexcept
		{
			return __atomic_fetch_or(&m_value, arg, order);
		}

		always_inline T fetch_xor(T arg, MemoryOrder order = memory_order_seq_cst) volatile noexcept
		{
			return __atomic_fetch_xor(&m_value, arg, order);
		}

		always_inline T operator=(T desired) volatile noexcept
		{
			store(desired);
			return desired;
		}

		always_inline operator T() const volatile noexcept
		{
			return load();
		}

		always_inline T operator++() volatile noexcept
		{
			return fetch_add(1) + 1;
		}

		always_inline T operator++(int) volatile noexcept
		{
			return fetch_add(1);
		}

		always_inline T operator--() volatile noexcept
		{
			return fetch_sub(1) + 1;
		}

		always_inline T operator--(int) volatile noexcept
		{
			return fetch_sub(1);
		}

		always_inline T operator+=(T arg) volatile noexcept
		{
			return fetch_add(arg) + arg;
		}

		always_inline T operator-=(T arg) volatile noexcept
		{
			return fetch_sub(arg) - arg;
		}

		always_inline T operator&=(T arg) volatile noexcept
		{
			return fetch_and(arg) & arg;
		}

		always_inline T operator|=(T arg) volatile noexcept
		{
			return fetch_or(arg) | arg;
		}

		always_inline T operator^=(T arg) volatile noexcept
		{
			return fetch_xor(arg) ^ arg;
		}
	};

	typedef atomic<bool> atomic_bool;

	typedef atomic<char> atomic_char;
	typedef atomic<signed char> atomic_schar;
	typedef atomic<unsigned char> atomic_uchar;

	typedef atomic<short> atomic_short;
	typedef atomic<unsigned short> atomic_ushort;

	typedef atomic<int> atomic_int;
	typedef atomic<unsigned int> atomic_uint;

	typedef atomic<long> atomic_long;
	typedef atomic<unsigned long> atomic_ulong;

	typedef atomic<long long> atomic_llong;
	typedef atomic<unsigned long long> atomic_ullong;

	typedef atomic<char16_t> atomic_char16_t;
	typedef atomic<char32_t> atomic_char32_t;
	typedef atomic<wchar_t> atomic_wchar_t;

	typedef atomic<int8_t> atomic_int8_t;
	typedef atomic<uint8_t> atomic_uint8_t;

	typedef atomic<int16_t> atomic_int16_t;
	typedef atomic<uint16_t> atomic_uint16_t;

	typedef atomic<int32_t> atomic_int32_t;
	typedef atomic<uint32_t> atomic_uint32_t;

	typedef atomic<int64_t> atomic_int64_t;
	typedef atomic<uint64_t> atomic_uint64_t;

	typedef atomic<int_least8_t> atomic_int_least8_t;
	typedef atomic<uint_least8_t> atomic_uint_least8_t;

	typedef atomic<int_least16_t> atomic_int_least16_t;
	typedef atomic<uint_least16_t> atomic_uint_least16_t;

	typedef atomic<int_least32_t> atomic_int_least32_t;
	typedef atomic<uint_least32_t> atomic_uint_least32_t;

	typedef atomic<int_least64_t> atomic_int_least64_t;
	typedef atomic<uint_least64_t> atomic_uint_least64_t;

	typedef atomic<int_fast8_t> atomic_int_fast8_t;
	typedef atomic<uint_fast8_t> atomic_uint_fast8_t;

	typedef atomic<int_fast16_t> atomic_int_fast16_t;
	typedef atomic<uint_fast16_t> atomic_uint_fast16_t;

	typedef atomic<int_fast32_t> atomic_int_fast32_t;
	typedef atomic<uint_fast32_t> atomic_uint_fast32_t;

	typedef atomic<int_fast64_t> atomic_int_fast64_t;
	typedef atomic<uint_fast64_t> atomic_uint_fast64_t;

	typedef atomic<intptr_t> atomic_intptr_t;
	typedef atomic<uintptr_t> atomic_uintptr_t;

	typedef atomic<size_t> atomic_size_t;

	typedef atomic<ptrdiff_t> atomic_ptrdiff_t;

	typedef atomic<intmax_t> atomic_intmax_t;
	typedef atomic<uintmax_t> atomic_uintmax_t;
} // namespace Kernel::LibK
