#pragma once

#include <stddef.h>

namespace Kernel
{
	class Console
	{
	public:
		virtual size_t get_width() const = 0;
		virtual size_t get_height() const = 0;

		virtual void scroll_up() = 0;
		virtual void scroll_down() = 0;

		virtual void put_char_at(size_t row, size_t column, char ch) = 0;
		virtual void clear(size_t from_row, size_t from_column, size_t to_row, size_t to_column) = 0;
	};
}