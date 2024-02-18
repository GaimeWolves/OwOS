#include <vga/textmode.hpp>

#include <stddef.h>
#include <stdint.h>

#include "logging/logger.hpp"
#include <arch/io.hpp>
#include <arch/spinlock.hpp>
#include <common_attributes.h>
#include <libk/kcstdio.hpp>
#include <vga/TextmodeBuffer.hpp>

#define ROWS 25
#define COLS 80

#define VGA_MEMORY 0xB8000

#define VGA_CONTROL_REG 0x3D4 // VGA control register port

#define VGA_CURSOR_POS_LOW_REG  0x0F // VGA cursor position low register index
#define VGA_CURSOR_POS_HIGH_REG 0x0E // VGA cursor position high register index

#define ColorToFourBit(color)   (uint8_t(color) & 0x0F)
#define FourBitToColor(fourBit) (Color(fourBit))

namespace Kernel::VGA::Textmode
{
	typedef struct vga_char_color
	{
		vga_char_color(const Color fg, const Color bg)
		    : fg(ColorToFourBit(fg)), bg(ColorToFourBit(bg))
		{
		}

		//Sadly c++ does not support bit-field Enums so we have to use uint8_t and ugly casts here
		uint8_t fg : 4;
		uint8_t bg : 4;

		void setFg(const Color color) { fg = ColorToFourBit(color); }
		void setBg(const Color color) { bg = ColorToFourBit(color); }
		Color getFg() { return FourBitToColor(fg); }
		Color getBg() { return FourBitToColor(bg); }
	} __packed vga_char_color_t;
	static_assert(sizeof(vga_char_color_t) == 1);

	typedef struct vga_char
	{
		vga_char(const uint8_t ch, const vga_char_color_t color)
		    : ch(ch), color(color)
		{
		}

		char ch;
		vga_char_color_t color;
	} __packed vga_char_t;
	static_assert(sizeof(vga_char_t) == 2);

	static inline vga_char_color_t vga_char_color(Color fg, Color bg);
	static inline vga_char_t vga_char(char ch, vga_char_color_t color);

	static void handle_scrolling();

	static void update_cursor();

	static uint8_t cursorX = 0, cursorY = 0;
	static vga_char_color_t color = vga_char_color(Color::WHITE, Color::BLACK);
	static Textmode::TextmodeBuffer<vga_char_t> buffer(VGA_MEMORY, ROWS, COLS);
	static Locking::Spinlock vga_lock;
	static Locking::Spinlock puts_lock;

	static IO::Port controlIndex(VGA_CONTROL_REG);
	static IO::Port controlData = controlIndex.offset(1);

	static bool initialized = false;

	static inline vga_char_color_t vga_char_color(Color fg, Color bg)
	{
		return vga_char_color_t(fg, bg);
	}

	static inline vga_char_t vga_char(char ch, vga_char_color_t color)
	{
		return vga_char_t(ch, color);
	}

	static void handle_scrolling()
	{
		if (cursorY < ROWS)
			return;

		for (uint8_t y = 0; y < ROWS - 1; y++)
		{
			for (uint8_t x = 0; x < COLS; x++)
			{
				buffer(x, y) = buffer(x, y + 1);
			}
		}

		for (uint8_t x = 0; x < COLS; x++)
		{
			buffer(x, ROWS - 1) = vga_char(' ', color);
		}

		cursorY--;
	}

	static void update_cursor()
	{
		uint16_t position = cursorY * COLS + cursorX;

		controlIndex.out((uint8_t)VGA_CURSOR_POS_LOW_REG);
		controlData.out((uint8_t)(position & 0xFF));
		controlIndex.out((uint8_t)VGA_CURSOR_POS_HIGH_REG);
		controlData.out((uint8_t)((position >> 8) & 0xFF));
	}

	void init()
	{
		for (uint8_t y = 0; y < ROWS; y++)
		{
			for (uint8_t x = 0; x < COLS; x++)
			{
				buffer(x, y) = vga_char(' ', color);
			}
		}

		cursorX = 0;
		cursorY = 0;

		initialized = true;

		log("VGA", "Text mode initialized");
	}

	bool is_initialized()
	{
		return initialized;
	}

	void putc(char ch)
	{
		if (!initialized)
			return;

		vga_lock.lock();
		if (ch == '\n')
		{
			cursorX = 0;
			cursorY++;
		}
		else
		{
			buffer(cursorX, cursorY) = vga_char(ch, color);
			cursorX++;

			if (cursorX >= COLS)
			{
				cursorX = 0;
				cursorY++;
			}
		}

		handle_scrolling();
		update_cursor();
		vga_lock.unlock();
	}

	void puts(const char *str)
	{
		if (!initialized)
			return;

		puts_lock.lock();
		for (size_t i = 0; str[i]; i++)
			putc(str[i]);
		puts_lock.unlock();
	}

	void set_color(Color fg, Color bg)
	{
		color = vga_char_color(fg, bg);
	}
} // namespace Kernel::VGA::Textmode
