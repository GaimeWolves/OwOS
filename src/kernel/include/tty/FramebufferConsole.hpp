#pragma once

#include <tty/Console.hpp>

namespace Kernel
{
	class FramebufferConsole final : public Console
	{
	public:
		static FramebufferConsole &instance()
		{
			static FramebufferConsole *instance{nullptr};

			if (!instance)
				instance = new FramebufferConsole();

			return *instance;
		}

		[[nodiscard]] virtual size_t get_width() const override;
		[[nodiscard]] virtual size_t get_height() const override;

		virtual void scroll_up() override;
		virtual void scroll_down() override;

		virtual void write_char(size_t row, size_t column, char ch) override;
		virtual void write_line(size_t row, char *buffer) override;
		virtual void write_region(size_t from_row, size_t from_column, size_t to_row, size_t to_column, char *buf) override;

		virtual void write_char(size_t row, size_t column, cell_t ch) override;
		virtual void write_line(size_t row, cell_t *buf) override;
		virtual void write_region(size_t from_row, size_t from_column, size_t to_row, size_t to_column, cell_t *buf) override;

		virtual void clear(size_t from_row, size_t from_column, size_t to_row, size_t to_column) override;
		virtual void clear() override;

		virtual void set_fg_color(uint32_t color) override;
		virtual void set_fg_color(ANSIColor color) override;
		virtual void set_bg_color(uint32_t color) override;
		virtual void set_bg_color(ANSIColor color) override;

		void set_cursor_at(size_t row, size_t column) override;
		void toggle_cursor(bool on) override;

	private:
		FramebufferConsole();

		void draw_cell(cell_t cell, size_t row, size_t column);
		void invalidate(size_t from_row, size_t from_column, size_t to_row, size_t to_column);

		void draw_cursor();
		void on_cursor_tick();

		uint32_t m_fg_color{0x00FFFFFF}, m_bg_color{0};
		size_t m_cursor_row{0}, m_cursor_column{0};
		bool m_cursor_on{true};
		bool m_cursor_tick_scheduled{false};
		cell_t *m_cells{nullptr};
		bool m_show_cursor{false};
	};
}
