#include <vga/textmode.hpp>

#include <stddef.h>
#include <stdint.h>

#include <arch/io.hpp>
#include <libk/kcstdio.hpp>
#include <memory/MMIO.hpp>

#define ROWS 25
#define COLS 80

#define VGA_MEMORY 0xB8000

#define VGA_CONTROL_REG 0x3D4 // VGA control register port

#define VGA_CURSOR_POS_LOW_REG  0x0F // VGA cursor position low register index
#define VGA_CURSOR_POS_HIGH_REG 0x0E // VGA cursor position high register index

namespace Kernel::VGA::Textmode
{
	typedef uint16_t vga_char_t;

	static inline uint8_t vga_char_color(Color fg, Color bg);
	static inline vga_char_t vga_char(uint8_t ch, uint8_t color);

	static void handle_scrolling();

	static void update_cursor();

	static uint8_t cursorX = 0, cursorY = 0;
	static uint8_t color = vga_char_color(Color::WHITE, Color::BLACK);
	static Memory::MMIO<vga_char_t> buffer(VGA_MEMORY, sizeof(vga_char_t) * ROWS * COLS);

	static IO::Port controlIndex(VGA_CONTROL_REG);
	static IO::Port controlData = controlIndex.offset(1);

	static inline uint8_t vga_char_color(Color fg, Color bg)
	{
		return (uint8_t)fg | ((uint8_t)bg << 4);
	}

	static inline vga_char_t vga_char(uint8_t ch, uint8_t color)
	{
		return (vga_char_t)ch | ((vga_char_t)color << 8);
	}

	static void handle_scrolling()
	{
		if (cursorY < ROWS)
			return;

		for (uint8_t y = 0; y < ROWS - 1; y++)
		{
			for (uint8_t x = 0; x < COLS; x++)
			{
				buffer[y * COLS + x] = buffer[(y + 1) * COLS + x];
			}
		}

		for (uint8_t x = 0; x < COLS; x++)
		{
			buffer[(ROWS - 1) * COLS + x] = vga_char(' ', color);
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
				buffer[y * COLS + x] = vga_char(' ', color);
			}
		}

		cursorX = 0;
		cursorY = 0;

		LibK::printf_check_msg(true, "VGA textmode");
	}

	void putc(const char ch)
	{
		if (ch == '\n')
		{
			cursorX = 0;
			cursorY++;
		}
		else
		{
			buffer[cursorY * COLS + cursorX] = vga_char((uint8_t)ch, color);
			cursorX++;

			if (cursorX >= COLS)
			{
				cursorX = 0;
				cursorY++;
			}
		}

		handle_scrolling();
		update_cursor();
	}

	void puts(const char *str)
	{
		for (size_t i = 0; str[i]; i++)
			putc(str[i]);
	}

	void set_color(Color fg, Color bg)
	{
		color = vga_char_color(fg, bg);
	}
} // namespace Kernel::VGA::Textmode