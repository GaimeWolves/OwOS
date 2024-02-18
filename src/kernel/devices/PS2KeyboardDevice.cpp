#include <devices/PS2KeyboardDevice.hpp>

#include <libk/kcctype.hpp>

#include <logging/logger.hpp>
#include <devices/KeyCode.hpp>

namespace Kernel
{
	char default_keycode_map[Key_Max] = {
	    0, '\033',
	    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\177',
	    0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
	    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
	    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, 0, 0, ' ', 0,
	};

	char default_keycode_map_shifted[Key_Max] = {
	    0, '\033',
	    '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\177',
	    0, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P','{', '}', '\n', 0,
	    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|',
	    'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, 0, 0, ' ', 0,
	};

	KeyCode default_scancode_map[] = {
	    Key_None,
	    Key_Escape,
	    Key_1,
	    Key_2,
	    Key_3,
	    Key_4,
	    Key_5,
	    Key_6,
	    Key_7,
	    Key_8,
	    Key_9,
	    Key_0,
	    Key_Minus,
	    Key_Equals,
	    Key_Backspace,
	    Key_Tab,
	    Key_Q,
	    Key_W,
	    Key_E,
	    Key_R,
	    Key_T,
	    Key_Y,
	    Key_U,
	    Key_I,
	    Key_O,
	    Key_P,
	    Key_LeftBracket,
	    Key_RightBracket,
	    Key_Return,
	    Key_LeftControl,
	    Key_A,
	    Key_S,
	    Key_D,
	    Key_F,
	    Key_G,
	    Key_H,
	    Key_J,
	    Key_K,
	    Key_L,
	    Key_Semicolon,
	    Key_Apostrophe,
	    Key_BackTick,
	    Key_LeftShift,
	    Key_Backslash,
	    Key_Z,
	    Key_X,
	    Key_C,
	    Key_V,
	    Key_B,
	    Key_N,
	    Key_M,
	    Key_Comma,
	    Key_Period,
	    Key_Slash,
	    Key_RightShift,
	    Key_Asterisk,
	    Key_LeftAlt,
	    Key_Space,
	    Key_CapsLock,
	    Key_F1,
	    Key_F2,
	    Key_F3,
	    Key_F4,
	    Key_F5,
	    Key_F6,
	    Key_F7,
	    Key_F8,
	    Key_F9,
	    Key_F10,
	    Key_NumLock,
	    Key_ScrollLock,
	    Key_Home,
	    Key_Up,
	    Key_PageUp,
	    Key_Left,
	    Key_None,
	    Key_Right,
	    Key_Plus,
	    Key_End,
	    Key_Down,
	    Key_PageDown,
	    Key_Insert,
	    Key_Delete,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_F11,
	    Key_F12,
	    Key_None,
	    Key_None,
	    Key_Super,
	    Key_None,
	    Key_Menu,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	};


	// TODO: Fill with correct data (for now only cursor keys)
	KeyCode default_e0_scancode_map[] = {
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_Up,
	    Key_None,
	    Key_None,
	    Key_Left,
	    Key_None,
	    Key_Right,
	    Key_None,
	    Key_None,
	    Key_Down,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	    Key_None,
	};

	void PS2KeyboardDevice::parse_modifiers(bool released, KeyCode key)
	{
		uint8_t bit = 0;

		switch (key)
		{
		case Key_LeftShift:
		case Key_RightShift:
			bit = KeyModifiers::Shift;
			break;
		case Key_LeftControl:
		case Key_RightControl:
			bit = KeyModifiers::Control;
			break;
		case Key_LeftAlt:
			bit = KeyModifiers::Alt;
			break;
		case Key_RightAlt:
			bit = KeyModifiers::AltGr;
			break;
		case Key_Super:
			bit = KeyModifiers::Super;
			break;
		case Key_CapsLock:
			if (released)
				m_modifiers ^= KeyModifiers::CapsLock;
			return;
		default:
			break;
		}

		if (released)
			m_modifiers &= ~bit;
		else
			m_modifiers |= bit;
	}

	void PS2KeyboardDevice::handle_interrupt(const CPU::interrupt_frame_t &)
	{
		auto scancode = m_input_buffer_port.in<uint8_t>();

		if (scancode == 0xe0)
		{
			m_e0_prefix = true;
			return;
		}

		bool released = scancode & 0x80;
		scancode &= 0x7f;

		KeyCode key = m_e0_prefix ? default_e0_scancode_map[scancode] : default_scancode_map[scancode];

		if (key == Key_None)
			return;

		parse_modifiers(released, key);

		m_e0_prefix = false;

		uint8_t modifiers = m_modifiers;

		if (!released)
			modifiers |= KeyModifiers::Pressed;

		bool shifted = 	modifiers & KeyModifiers::Shift || modifiers & KeyModifiers::CapsLock;
		char code_point = shifted ? default_keycode_map_shifted[key] : default_keycode_map[key];

		key_event_t event {
		    .key = key,
		    .scancode = scancode,
		    .modifiers = modifiers,
		    .code_point = code_point
		};

		// TODO: only for testing purposes

		if (event.code_point == '\\')
			event.code_point = '\025';

		push(event);
	}

	void PS2KeyboardDevice::enable()
	{
		enable_irq();
	}
}
