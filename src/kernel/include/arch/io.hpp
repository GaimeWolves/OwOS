#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef ARCH_i686
#	include <arch/i686/io.hpp>
#endif

namespace Kernel::IO
{
	template <typename T>
	inline __attribute__((always_inline)) void out(size_t port, T value);

	template <typename T>
	inline __attribute__((always_inline)) T in(size_t port);

	class Port
	{
	public:
		Port() = default;
		Port(size_t address) : m_address(address) {}

		Port offset(size_t offset) const { return Port(m_address + offset); }

		size_t get() const { return m_address; }
		void set(size_t address) { m_address = address; }

		template <typename T>
		inline __attribute__((always_inline)) void out(T value) { IO::out<T>(m_address, value); }

		template <typename T>
		inline __attribute__((always_inline)) T in() { return IO::in<T>(m_address); }

		bool operator==(const Port &other) const { return m_address == other.m_address; }
		bool operator!=(const Port &other) const { return m_address != other.m_address; }
		bool operator>(const Port &other) const { return m_address > other.m_address; }
		bool operator>=(const Port &other) const { return m_address >= other.m_address; }
		bool operator<(const Port &other) const { return m_address < other.m_address; }
		bool operator<=(const Port &other) const { return m_address <= other.m_address; }

	private:
		size_t m_address{0};
	};

} // namespace Kernel::IO
