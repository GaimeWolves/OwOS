#pragma once

#include <stddef.h>
#include <stdint.h>

#include <common_attributes.h>

#ifdef ARCH_i686
#	include <arch/i686/io.hpp>
#else
#	error No arch-specific io.hpp included
#endif

namespace Kernel::IO
{
	template <typename T>
	always_inline void out(port_t port, T value);

	template <typename T>
	always_inline T in(port_t port);

	class Port
	{
	public:
		Port() = default;
		Port(port_t address) : m_address(address) {}

		Port offset(port_t offset) const { return Port(m_address + offset); }

		port_t get() const { return m_address; }
		void set(port_t address) { m_address = address; }

		template <typename T>
		always_inline void out(T value) { IO::out<T>(m_address, value); }

		template <typename T>
		always_inline T in() { return IO::in<T>(m_address); }

		bool operator==(const Port &other) const { return m_address == other.m_address; }
		bool operator!=(const Port &other) const { return m_address != other.m_address; }
		bool operator>(const Port &other) const { return m_address > other.m_address; }
		bool operator>=(const Port &other) const { return m_address >= other.m_address; }
		bool operator<(const Port &other) const { return m_address < other.m_address; }
		bool operator<=(const Port &other) const { return m_address <= other.m_address; }

	private:
		port_t m_address{0};
	};

} // namespace Kernel::IO
