#pragma once

#include <stddef.h>
#include <stdint.h>

#include <libk/kstring.hpp>

#include <arch/io.hpp>
#include <devices/CharacterDevice.hpp>

#include <multiboot.h>

namespace Kernel
{
	class FramebufferDevice : public CharacterDevice
	{
	public:
		static FramebufferDevice &instance()
		{
			static FramebufferDevice *instance{nullptr};

			if (!instance)
				instance = new FramebufferDevice();

			return *instance;
		}

		void configure(multiboot_info_t *multiboot_info);
		[[nodiscard]] bool is_initialized() const { return m_framebuffer; }

		LibK::StringView name() override { return LibK::StringView("fbdev"); };

		[[nodiscard]] size_t size() override { return 0; }

		size_t read(size_t offset, size_t bytes, char *buffer) override;
		size_t write(size_t offset, size_t bytes, char *buffer) override;

		void blit(uint32_t *data, bool direct, size_t x, size_t y, size_t width, size_t height);
		void blit_character(uint8_t *data, bool direct, size_t x, size_t y, size_t width, size_t height, size_t stride, uint32_t fg_color, uint32_t bg_color);

		void clear_screen(bool direct, size_t from_x, size_t from_y, size_t to_x, size_t to_y, uint32_t color);

		void scroll_vertical(int amount);

		[[nodiscard]] size_t get_width() const { return m_width; }
		[[nodiscard]] size_t get_height() const { return m_height; }

	protected:
		[[nodiscard]] bool can_open_for_read() const override { return true; };
		[[nodiscard]] bool can_open_for_write() const override { return true; };

	private:
		FramebufferDevice()
		    : CharacterDevice(0, 0)
		{
		}

		void blit_screen();

		uint8_t *m_offscreen_buffer;
		size_t m_offset;

		uint8_t *m_framebuffer;
		size_t m_size;
		size_t m_width;
		size_t m_height;
	};
} // namespace Kernel::Devices
