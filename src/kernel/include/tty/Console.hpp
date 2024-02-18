#pragma once

#include <stddef.h>
#include <stdint.h>

#include <tty/ANSIColor.hpp>

namespace Kernel
{
	class Console
	{
	public:
		typedef struct __cell_t
		{
			char ch;
			uint32_t fg_color;
			uint32_t bg_color;
		} cell_t;

		static constexpr cell_t default_cell = { ' ', ANSIColorToRGBA(ANSIColor::BrightWhite), ANSIColorToRGBA(ANSIColor::Black) };

		virtual size_t get_width() const = 0;
		virtual size_t get_height() const = 0;

		virtual void scroll_up() = 0;
		virtual void scroll_down() = 0;

		virtual void write_char(size_t row, size_t column, char ch) = 0;
		virtual void write_line(size_t row, char *buf) = 0;
		virtual void write_region(size_t from_row, size_t from_column, size_t to_row, size_t to_column, char *buf) = 0;

		virtual void write_char(size_t row, size_t column, cell_t ch) = 0;
		virtual void write_line(size_t row, cell_t *buf) = 0;
		virtual void write_region(size_t from_row, size_t from_column, size_t to_row, size_t to_column, cell_t *buf) = 0;

		virtual void clear(size_t from_row, size_t from_column, size_t to_row, size_t to_column) = 0;
		virtual void clear() = 0;

		virtual void set_fg_color(uint32_t color) = 0;
		virtual void set_fg_color(ANSIColor color) = 0;
		virtual void set_bg_color(uint32_t color) = 0;
		virtual void set_bg_color(ANSIColor color) = 0;

		virtual void set_cursor_at(size_t row, size_t column) = 0;
		virtual void toggle_cursor(bool on) = 0;
	};
}
