#include <tty/FramebufferConsole.hpp>

#include <devices/FramebufferDevice.hpp>
#include <tty/psf.hpp>

namespace Kernel
{
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
		FramebufferDevice::instance().scroll_vertical((int)PSFFont::get_embedded().get_height());
	}

	void FramebufferConsole::scroll_down()
	{
		FramebufferDevice::instance().scroll_vertical(-(int)PSFFont::get_embedded().get_height());
	}

	void FramebufferConsole::put_char_at(size_t row, size_t column, char ch)
	{
		auto glyph = PSFFont::get_embedded().get_glyph(ch);

		if (!glyph.bitmap) {
			return;
		}

		size_t x = column * glyph.width;
		size_t y = row * glyph.height;

		FramebufferDevice::instance().blit_character(glyph.bitmap, true, x, y, glyph.width, glyph.height, glyph.stride, 0xFFFFFFFF, 0);
	}

	void FramebufferConsole::clear(size_t from_row, size_t from_column, size_t to_row, size_t to_column)
	{
		if (from_column != 0)
		{
			for (size_t col = from_column; col < get_width(); col++)
			{
				put_char_at(from_row, col, ' ');
			}

			from_row++;
		}

		if (to_column != get_width())
		{
			for (size_t col = 0; col < to_column; col++)
			{
				put_char_at(to_row, col, ' ');
			}

			to_row--;
		}

		size_t height = PSFFont::get_embedded().get_height();

		FramebufferDevice::instance().clear_screen(true, 0, from_row * height, FramebufferDevice::instance().get_width(), (to_row + 1) * height, 0);
	}
} // namespace Kernel