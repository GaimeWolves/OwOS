#include <tty/FramebufferConsole.hpp>

#include <devices/FramebufferDevice.hpp>
#include <tty/psf.hpp>

#include <time/EventManager.hpp>
#include <tty/ANSIColor.hpp>

namespace Kernel
{
	FramebufferConsole::FramebufferConsole()
	{
		m_cells = static_cast<cell_t *>(kcalloc(sizeof(cell_t) * get_width() * get_height(), alignof(cell_t)));
		set_fg_color(ANSIColor::BrightWhite);
		set_bg_color(ANSIColor::Black);
		on_cursor_tick();
	}

	void FramebufferConsole::draw_cursor()
	{
		cell_t cell = m_cells[m_cursor_row * get_width() + m_cursor_column];

		if (m_show_cursor)
		{
			uint32_t fg = cell.fg_color;
			cell.fg_color = cell.bg_color;
			cell.bg_color = fg;
		}

		draw_cell(cell, m_cursor_row, m_cursor_column);
	}

	void FramebufferConsole::on_cursor_tick()
	{
		m_cursor_tick_scheduled = false;
		m_show_cursor = !m_show_cursor;

		draw_cursor();

		if (m_cursor_on)
		{
			m_cursor_tick_scheduled = true;
			Time::EventManager::instance().schedule_event([&]() { this->on_cursor_tick(); }, Time::from_milliseconds(200), true);
		}
	}

	size_t FramebufferConsole::get_width() const
	{
		return FramebufferDevice::instance().get_width() / PSFFont::get_embedded().get_width();
	}

	size_t FramebufferConsole::get_height() const
	{
		return FramebufferDevice::instance().get_height() / PSFFont::get_embedded().get_height();
	}

	void FramebufferConsole::scroll_up()
	{
		bool on = m_cursor_on;
		toggle_cursor(false);
		FramebufferDevice::instance().scroll_vertical((int)PSFFont::get_embedded().get_height());
		toggle_cursor(on);
	}

	void FramebufferConsole::scroll_down()
	{
		bool on = m_cursor_on;
		toggle_cursor(false);
		FramebufferDevice::instance().scroll_vertical(-(int)PSFFont::get_embedded().get_height());
		toggle_cursor(on);
	}

	void FramebufferConsole::write_char(size_t row, size_t column, char ch)
	{
		cell_t cell = { ch, m_fg_color, m_bg_color };
		m_cells[row * get_width() + column] = cell;
		draw_cell(cell, row, column);
	}

	void FramebufferConsole::write_line(size_t row, char *buffer)
	{
		for (size_t column = 0; column < get_width(); column++)
		{
			cell_t cell = { buffer[column], m_fg_color, m_bg_color };
			m_cells[row * get_width() + column] = cell;
			draw_cell(cell, row, column);
		}
	}

	void FramebufferConsole::write_region(size_t from_row, size_t from_column, size_t to_row, size_t to_column, char *buffer)
	{
		size_t from_index = from_row * get_width() + from_column;
		size_t to_index = to_row * get_width() + to_column;

		for (size_t i = 0; i <= to_index - from_index; i++)
		{
			cell_t cell = { buffer[i], m_fg_color, m_bg_color };
			m_cells[from_index + i] = cell;
			draw_cell(cell, from_row + i / get_width(), (from_column + i) % get_width());
		}
	}

	void FramebufferConsole::write_char(size_t row, size_t column, cell_t ch)
	{
		m_cells[row * get_width() + column] = ch;
		draw_cell(ch, row, column);
	}

	void FramebufferConsole::write_line(size_t row, cell_t *buf)
	{
		memmove(m_cells + row * get_width(), buf, sizeof(cell_t) * get_width());
		invalidate(row, 0, row, get_width() - 1);
	}

	void FramebufferConsole::write_region(size_t from_row, size_t from_column, size_t to_row, size_t to_column, cell_t *buf)
	{
		size_t from_index = from_row * get_width() + from_column;
		size_t to_index = to_row * get_width() + to_column;
		memmove(m_cells + from_index, buf, sizeof(cell_t) * (to_index - from_index + 1));
		invalidate(from_row, from_column, to_row, to_column);
	}

	void FramebufferConsole::draw_cell(cell_t cell, size_t row, size_t column)
	{
		char ch = cell.ch;

		if (ch == 0)
			ch = ' ';

		auto glyph = PSFFont::get_embedded().get_glyph(ch);

		if (!glyph.bitmap) {
			return;
		}

		size_t x = column * glyph.width;
		size_t y = row * glyph.height;

		FramebufferDevice::instance().blit_character(glyph.bitmap, true, x, y, glyph.width, glyph.height, glyph.stride, cell.fg_color, cell.bg_color);
	}

	void FramebufferConsole::invalidate(size_t from_row, size_t from_column, size_t to_row, size_t to_column)
	{
		size_t from_index = from_row * get_width() + from_column;
		size_t to_index = to_row * get_width() + to_column;

		for (size_t i = from_index; i <= to_index; i++)
			draw_cell(m_cells[i], from_row + i / get_width(), (from_column + i) % get_width());
	}

	void FramebufferConsole::clear(size_t from_row, size_t from_column, size_t to_row, size_t to_column)
	{
		if (from_row == to_row)
		{
			for (size_t col = from_column; col <= to_column; col++)
			{
				write_char(from_row, col, ' ');
			}

			return;
		}

		if (from_column != 0)
		{
			for (size_t col = from_column; col < get_width(); col++)
			{
				write_char(from_row, col, ' ');
			}

			from_row++;
		}

		if (to_column != get_width() - 1)
		{
			for (size_t col = 0; col <= to_column; col++)
			{
				write_char(to_row, col, ' ');
			}

			to_row--;
		}

		for (size_t i = from_row * get_width() + from_column; i < to_row * get_width() + to_column; i++)
		{
			m_cells[i] = { ' ', m_fg_color, m_bg_color };
		}

		size_t height = PSFFont::get_embedded().get_height();

		FramebufferDevice::instance().clear_screen(true, 0, from_row * height, FramebufferDevice::instance().get_width(), (to_row + 1) * height, m_bg_color);
	}

	void FramebufferConsole::clear()
	{
		clear(0, 0, get_height() - 1, get_width() - 1);
	}

	void FramebufferConsole::set_fg_color(uint32_t color)
	{
		m_fg_color = color;
	}

	void FramebufferConsole::set_fg_color(ANSIColor color)
	{
		m_fg_color = ANSIColorToRGBA(color);
	}

	void FramebufferConsole::set_bg_color(uint32_t color)
	{
		m_bg_color = color;
	}

	void FramebufferConsole::set_bg_color(ANSIColor color)
	{
		m_bg_color = ANSIColorToRGBA(color);
	}

	void FramebufferConsole::set_cursor_at(size_t row, size_t column)
	{
		assert(row < get_height() && column < get_width());

		draw_cell(m_cells[m_cursor_row * get_width() + m_cursor_column], m_cursor_row, m_cursor_column);

		m_cursor_column = column;
		m_cursor_row = row;

		draw_cursor();
	}

	void FramebufferConsole::toggle_cursor(bool on)
	{
		if (on && !m_cursor_tick_scheduled)
			on_cursor_tick();

		m_cursor_on = on;

		if (!m_cursor_on)
			draw_cell(m_cells[m_cursor_row * get_width() + m_cursor_column], m_cursor_row, m_cursor_column);
	}
} // namespace Kernel