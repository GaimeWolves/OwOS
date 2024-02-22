#pragma once

#include <new>

namespace Kernel::LibK
{

	template <typename T>
	class allocator
	{
	public:
		typedef T value_type;
		typedef std::true_type propagate_on_container_copy_assignment;
		typedef std::true_type propagate_on_container_move_assignment;
		typedef std::true_type propagate_on_container_swap;
		[[deprecated]] typedef std::true_type is_always_equal;

		constexpr allocator() noexcept = default;
		constexpr allocator(const allocator &) noexcept = default;
		constexpr allocator &operator=(const allocator &) noexcept = default;
		constexpr allocator(allocator &&) noexcept = default;
		constexpr allocator &operator=(allocator &&) noexcept = default;

		template <class U>
		constexpr allocator(const allocator<U> &) noexcept {};

		constexpr ~allocator() = default;

		[[nodiscard]] constexpr T *allocate(std::size_t n) { return static_cast<T *>(::operator new(n * sizeof(T), static_cast<std::align_val_t>(alignof(T)))); }
		void deallocate(T *p, std::size_t n) { ::operator delete(p, n * sizeof(T), static_cast<std::align_val_t>(alignof(T))); }

	private:
	};

	template <class T, class U>
	constexpr bool operator==(const allocator<T> &, const allocator<U> &) noexcept { return true; }

	template <class T, class U>
	constexpr bool operator!=(const allocator<T> &, const allocator<U> &) noexcept { return false; }

} // namespace Kernel::LibK
