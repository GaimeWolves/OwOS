#pragma once

#include <stddef.h>
#include <stdint.h>

namespace Kernel::IO
{
	typedef uint16_t port_t;

	inline __attribute__((always_inline)) void out8(port_t port, uint8_t value)
	{
		asm volatile("outb %0, %1" ::"a"(value), "Nd"(port));
	}

	inline __attribute__((always_inline)) void out16(port_t port, uint16_t value)
	{
		asm volatile("outw %0, %1" ::"a"(value), "Nd"(port));
	}

	inline __attribute__((always_inline)) void out32(port_t port, uint32_t value)
	{
		asm volatile("outl %0, %1" ::"a"(value), "Nd"(port));
	}

	template <typename T>
	inline __attribute__((always_inline)) void out(port_t port, T value)
	{
		if constexpr (sizeof(T) == 4)
		{
			out32(port, value);
			return;
		}

		if constexpr (sizeof(T) == 2)
		{
			out16(port, value);
			return;
		}

		if constexpr (sizeof(T) == 1)
		{
			out8(port, value);
			return;
		}
	}

	inline __attribute__((always_inline)) uint8_t in8(port_t port)
	{
		uint8_t value;

		asm volatile("inb %1, %0"
		             : "=a"(value)
		             : "Nd"(port));

		return value;
	}

	inline __attribute__((always_inline)) uint16_t in16(port_t port)
	{
		uint16_t value;

		asm volatile("inw %1, %0"
		             : "=a"(value)
		             : "Nd"(port));

		return value;
	}

	inline __attribute__((always_inline)) uint32_t in32(port_t port)
	{
		uint32_t value;

		asm volatile("inl %1, %0"
		             : "=a"(value)
		             : "Nd"(port));

		return value;
	}

	template <typename T>
	inline __attribute__((always_inline)) T in(port_t port)
	{
		if constexpr (sizeof(T) == 4)
			return in32(port);

		if constexpr (sizeof(T) == 2)
			return in16(port);

		if constexpr (sizeof(T) == 1)
			return in8(port);
	}

} // namespace Kernel::IO
