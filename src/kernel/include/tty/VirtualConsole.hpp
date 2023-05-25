#pragma once

#include <devices/KeyboardDevice.hpp>
#include <tty/Terminal.hpp>
#include <tty/TTY.hpp>
#include <tty/FramebufferConsole.hpp>

namespace Kernel
{
	class VirtualConsole final
	    : public TTY
	    , private Terminal
	    , private KeyboardListener
	{
	public:
		static VirtualConsole &get_current()
		{
			static VirtualConsole *instance = nullptr;

			if (!instance)
				instance = new VirtualConsole(0, 0);

			return *instance;
		}

		static void initialize()
		{
			// TODO: Have a HID manager manage this
			static PS2KeyboardDevice *keyboard = nullptr;

			if (!keyboard)
				keyboard = new PS2KeyboardDevice(0, 0);

			FramebufferConsole::instance().clear();
			keyboard->register_listener(&get_current());
			keyboard->enable();
		}

		LibK::StringView name() override { return LibK::StringView("tty0"); };

	private:
		VirtualConsole(size_t major, size_t minor)
		    : TTY(major, minor)
			, Terminal(&FramebufferConsole::instance())
		{
		}

		void echo(char ch) override { Terminal::echo(ch); }
		void unecho() override { Terminal::unecho(); };

		void handle_key_event(key_event_t event) override
		{
			LibK::string result = parse_key_event(event);
			for (auto ch : result)
				emit(ch);
		}
	};
}