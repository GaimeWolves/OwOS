#ifndef IO_H
#define IO_H 1

#include <stddef.h>
#include <stdint.h>

namespace Kernel::IO
{

	void out8(size_t port, uint8_t value);
	void out16(size_t port, uint16_t value);
	void out32(size_t port, uint32_t value);

	template <typename T>
	inline __attribute__((always_inline)) void out(size_t port, T value)
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

	uint8_t in8(size_t port);
	uint16_t in16(size_t port);
	uint32_t in32(size_t port);

	template <typename T>
	inline __attribute__((always_inline)) T in(size_t port)
	{
		if constexpr (sizeof(T) == 4)
			return in32(port);

		if constexpr (sizeof(T) == 2)
			return in16(port);

		if constexpr (sizeof(T) == 1)
			return in8(port);
	}

	class Port
	{
	public:
		Port() = default;
		Port(size_t address)
			: m_address(address)
		{
		}

		Port offset(size_t offset) const { return Port(m_address + offset); }

		size_t get() const { return m_address; }
		void set(size_t address) { m_address = address; }

		template <typename T>
		inline __attribute__((always_inline)) void out(T value) { IO::out<T>(m_address, value); }

		template <typename T>
		inline __attribute__((always_inline)) T in() { IO::in<T>(m_address); }

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

#endif // IO_H