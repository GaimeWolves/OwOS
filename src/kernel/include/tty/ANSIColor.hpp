#pragma once

#include <stdint.h>

namespace Kernel
{
	enum class ANSIColor
	{
		Black = 0,
		Red = 1,
		Green = 2,
		Yellow = 3,
		Blue = 4,
		Magenta = 5,
		Cyan = 6,
		White = 7,
		BrightBlack = 8,
		BrightRed = 9,
		BrightGreen = 10,
		BrightYellow = 11,
		BrightBlue = 12,
		BrightMagenta = 13,
		BrightCyan = 14,
		BrightWhite = 15,
	};

	inline uint32_t ANSIColorToRGBA(ANSIColor color)
	{
		static const uint32_t color_lut[] = {
			0x00000000,
			0x00AA0000,
			0x0000AA00,
			0x00AA5500,
			0x000000AA,
			0x00AA00AA,
			0x0000AAAA,
			0x00AAAAAA,
			0x00555555,
			0x00FF5555,
			0x0055FF55,
			0x00FFFF55,
			0x005555FF,
			0x00FF55FF,
			0x0055FFFF,
			0x00FFFFFF,
		};

		return color_lut[(int)color];
	}
}