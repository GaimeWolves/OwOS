#include <devices/FramebufferDevice.hpp>

#include <libk/kstring.hpp>

namespace Kernel
{
	// TODO: Put separate double buffer in RAM for efficient scrolling and have kernel thread periodically blit the screen if dirty
	void FramebufferDevice::configure(multiboot_info_t *multiboot_info)
	{
		if (!(multiboot_info->flags & MULTIBOOT_INFO_FRAMEBUFFER_INFO))
		{
			m_size = 0;
			m_framebuffer = nullptr;
			return;
		}

		m_width = multiboot_info->framebuffer_width;
		m_height = multiboot_info->framebuffer_height;
		m_size = multiboot_info->framebuffer_bpp * m_width * m_height / CHAR_BIT;

		m_offset = 0;

		Memory::mapping_config_t config = { };
		config.caching_mode = Memory::CachingMode::WriteCombining;
		config.writeable = true;
		config.readable = true;
		config.userspace = false;
		auto region = Memory::VirtualMemoryManager::instance().map_region(multiboot_info->framebuffer_addr, m_size, config);
		m_framebuffer = (uint8_t *)region.virt_region().pointer();

		auto offscreen_region = Memory::VirtualMemoryManager::instance().allocate_region(m_size * 2, config);
		auto phys_start = offscreen_region.phys_address;
		auto virt_start = offscreen_region.virt_address;
		m_offscreen_buffer = (uint8_t *)offscreen_region.virt_region().pointer();
		Memory::VirtualMemoryManager::instance().free(offscreen_region);

		// Emulate circular buffer using two consecutive maps of the framebuffer
		Memory::VirtualMemoryManager::instance().map_region_at(phys_start, virt_start, m_size, config);
		Memory::VirtualMemoryManager::instance().map_region_at(phys_start, virt_start + m_size, m_size, config);

		memset(m_offscreen_buffer, 0, m_size);
		memset(m_framebuffer, 0, m_size);
	}

	size_t FramebufferDevice::read(size_t offset, size_t bytes, char *buffer)
	{
		if (!is_initialized())
			return 0;

		if (offset >= m_size || bytes > m_size || offset + bytes > m_size)
			return 0;

		memcpy(buffer, m_framebuffer + offset, bytes);

		return bytes;
	}

	size_t FramebufferDevice::write(size_t offset, size_t bytes, char *buffer)
	{
		if (!is_initialized())
			return 0;

		if (offset >= m_size || bytes > m_size || offset + bytes > m_size)
			return 0;

		memcpy(m_framebuffer + offset, buffer, bytes);

		return bytes;
	}

	static void do_blit(uint32_t *data, uint32_t *buffer, size_t width, size_t height, size_t buffer_width)
	{
		while (height--)
		{
			for (size_t x_off = 0; x_off < width; x_off++)
			{
				buffer[x_off] = *data++;
			}

			buffer += buffer_width;
		}
	}

	static void do_blit_char(uint8_t *data, uint32_t *buffer, size_t width, size_t height, size_t stride, uint32_t fg_color, uint32_t bg_color, size_t buffer_width)
	{
		while (height--)
		{
			for (size_t x_off = 0; x_off < width; x_off++)
			{
				if (data[x_off / CHAR_BIT] & (0x80 >> (x_off % CHAR_BIT)))
					buffer[x_off] = fg_color;
				else
					buffer[x_off] = bg_color;
			}

			data += stride / CHAR_BIT;
			buffer += buffer_width;
		}
	}

	void FramebufferDevice::blit_screen()
	{
		if (!is_initialized())
			return;

		memcpy(m_framebuffer, m_offscreen_buffer + m_offset, m_size);
	}

	void FramebufferDevice::blit(uint32_t *data, bool direct, size_t x, size_t y, size_t width, size_t height)
	{
		if (!is_initialized())
			return;

		size_t offset = (y * m_width + x) * sizeof(uint32_t);
		auto *offscreen_position = reinterpret_cast<uint32_t *>(m_offscreen_buffer + m_offset + offset);
		auto *screen_position = reinterpret_cast<uint32_t *>(m_framebuffer + offset);

		if (direct)
			do_blit(data, screen_position, width, height, m_width);

		do_blit(data, offscreen_position, width, height, m_width);
	}

	void FramebufferDevice::blit_character(uint8_t *data, bool direct, size_t x, size_t y, size_t width, size_t height, size_t stride, uint32_t fg_color, uint32_t bg_color)
	{
		if (!is_initialized())
			return;

		size_t offset = (y * m_width + x) * sizeof(uint32_t);
		auto *offscreen_position = reinterpret_cast<uint32_t *>(m_offscreen_buffer + m_offset + offset);
		auto *screen_position = reinterpret_cast<uint32_t *>(m_framebuffer + offset);

		if (direct)
			do_blit_char(data, screen_position, width, height, stride, fg_color, bg_color, m_width);

		do_blit_char(data, offscreen_position, width, height, stride, fg_color, bg_color, m_width);
	}

	void FramebufferDevice::clear_screen(bool direct, size_t from_x, size_t from_y, size_t to_x, size_t to_y, uint32_t color)
	{
		if (!is_initialized())
			return;

		size_t start = (from_y * m_width + from_x) * sizeof(uint32_t);
		size_t end = (to_y * m_width + to_x) * sizeof(uint32_t);

		if (direct)
			memset32(m_framebuffer + start, color, (end - start) / sizeof(uint32_t));

		memset32(m_offscreen_buffer + m_offset + start, color, (end - start) / sizeof(uint32_t));
	}

	void FramebufferDevice::scroll_vertical(int amount)
	{
		if (!is_initialized())
			return;

		if (!amount)
			return;

		int count = amount * (int)m_width * sizeof(uint32_t);

		if (count < 0 && -count > (int)m_offset) {
			count += (int)m_offset;
			m_offset = m_size + count;
		} else {
			m_offset = (m_offset + count) % m_size;
		}

		blit_screen();
	}
}