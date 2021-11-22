#pragma once

#include <memory/MMIO.hpp>
#include <vga/textmode.hpp>

namespace Kernel::VGA::Textmode
{
	template <typename T>
	class TextmodeBuffer
	{
	private:
		Memory::MMIO<T> buffer;
		const uint8_t maxRows;
		const uint8_t maxCols;

	public:
		TextmodeBuffer(uintptr_t address, uint8_t rows, uint8_t cols)
		    : buffer(address, sizeof(T) * rows * cols), maxRows(rows), maxCols(cols)
		{
		}

		T &operator()(uint8_t col, uint8_t row)
		{
			return buffer[row * maxCols + col];
		}
	};
} // namespace Kernel::VGA::Textmode