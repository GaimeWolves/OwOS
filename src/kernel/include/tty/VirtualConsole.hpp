#pragma once

#include <devices/KeyboardDevice.hpp>
#include <tty/FramebufferConsole.hpp>
#include <tty/TTY.hpp>
#include <tty/Terminal.hpp>
#include <processes/Process.hpp>

#include "../../userland/libc/signal.h"

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

		[[nodiscard]] Process *get_controlling_process() const { return m_controlling_process; }
		void set_controlling_process(Process *process) { m_controlling_process = process; }

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
			// TODO: handle special keycodes and combinations like Ctrl^C

			if (event.key == Key_C && (event.modifiers & (~CapsLock)) == (Pressed | Control))
			{
				if (!m_controlling_process)
					return;

				m_controlling_process->send_signal(SIGINT);
				return;
			}

			LibK::string result = parse_key_event(event);
			for (auto ch : result)
				emit(ch);
		}

		Process *m_controlling_process{nullptr};
	};
}