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

		virtual void put_char_at(size_t row, size_t column, char ch) override;
		virtual void clear(size_t from_row, size_t from_column, size_t to_row, size_t to_column) override;
	};
}