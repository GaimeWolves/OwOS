#include <tty/Terminal.hpp>

#include <libk/kcstdlib.hpp>

#include <logging/logger.hpp>

#define NOT_IMPLEMENTED() unimplemented_control_sequence(__PRETTY_FUNCTION__)

#define ENSURE_ARG_COUNT(n) arguments.resize(n, 0)

namespace Kernel
{
	static void unimplemented_control_sequence(const char *name)
	{
		log("Terminal", "Unimplemented control sequence at %s", name);
	}

	void Terminal::echo(char ch)
	{
		switch (ch)
		{
		case '\a':
			BEL();
			return;
		case '\b':
			BS();
			return;
		case '\t':
			HT();
			return;
		case '\f':
			FF();
			return;
		case '\n':
			LF();
			return;
		case '\r':
			CR();
			return;
		case '\e':
			ESC();
			m_current_escape_sequence += ch;
			return;
		default:
			break;
		}

		if (m_sequence_state != SequenceState::None)
			m_current_escape_sequence += ch;

		if (m_sequence_state == SequenceState::CSISequence)
		{
			read_csi_sequence(ch);
			return;
		}

		if (m_sequence_state == SequenceState::FeSequence)
		{
			read_fe_sequence(ch);
			return;
		}

		if (isascii(ch))
		{
			m_console->put_char_at(m_cursor_row, m_cursor_column, ch);
			advance();
			m_console->set_cursor_at(m_cursor_row, m_cursor_column);
		}
		else
		{
			log("Terminal", "Ignoring %c 0x%x", ch, ch);
		}
	}

	void Terminal::reset_sequence()
	{
		log("Terminal", "Escape sequence: %s", m_current_escape_sequence.c_str());
		m_current_escape_sequence = "";
		m_sequence_state = SequenceState::None;
	}

	void Terminal::read_fe_sequence(char ch)
	{
		switch (ch)
		{
		case 'M':
			RI();
			break;
		case 'N':
			SS2();
			break;
		case 'O':
			SS3();
			break;
		case 'P':
			DCS();
			break;
		case '[':
			CSI();
			return;
		case '\\':
			ST();
			break;
		case ']':
			OSC();
			break;
		case 'X':
			SOS();
			break;
		case '^':
			PM();
			break;
		case '_':
			APC();
			break;
		default:
			NOT_IMPLEMENTED();
			break;
		}

		reset_sequence();
	}

	void Terminal::read_csi_sequence(char ch)
	{
		if (m_csi_state == CSIState::CSISequenceParams)
		{
			if (ch >= 0x30 && ch < 0x40)
				return;

			m_csi_state = CSIState::CSISequenceIntermediate;
		}

		if (m_csi_state == CSIState::CSISequenceIntermediate)
		{
			if (ch >= 0x20 && ch < 0x30)
				return;

			m_csi_state = CSIState::CSISequenceFinal;
		}

		LibK::vector<int> arguments = parse_csi_arguments();

		switch (ch)
		{
		case 'A':
			ENSURE_ARG_COUNT(1);
			CUU(arguments[0]);
			break;
		case 'B':
			ENSURE_ARG_COUNT(1);
			CUD(arguments[0]);
			break;
		case 'C':
			ENSURE_ARG_COUNT(1);
			CUF(arguments[0]);
			break;
		case 'D':
			ENSURE_ARG_COUNT(1);
			CUB(arguments[0]);
			break;
		case 'E':
			ENSURE_ARG_COUNT(1);
			CNL(arguments[0]);
			break;
		case 'F':
			ENSURE_ARG_COUNT(1);
			CPL(arguments[0]);
			break;
		case 'G':
			ENSURE_ARG_COUNT(1);
			CHA(arguments[0]);
			break;
		case 'H':
			ENSURE_ARG_COUNT(2);
			CUP(arguments[0], arguments[1]);
			break;
		case 'J':
			ENSURE_ARG_COUNT(1);
			ED(arguments[0]);
			break;
		case 'K':
			ENSURE_ARG_COUNT(1);
			EL(arguments[0]);
			break;
		case 'S':
			ENSURE_ARG_COUNT(1);
			SU(arguments[0]);
			break;
		case 'T':
			ENSURE_ARG_COUNT(1);
			SD(arguments[0]);
			break;
		case 'd':
			ENSURE_ARG_COUNT(1);
			VPA(arguments[0]);
			break;
		case 'f':
			ENSURE_ARG_COUNT(2);
			HVP(arguments[0], arguments[1]);
			break;
		case 'm':
			ENSURE_ARG_COUNT(1);
			SGR(arguments[0]);
			break;
		case 'i':
			ENSURE_ARG_COUNT(1);
			AUX(arguments[0]);
			break;
		case 'n':
			ENSURE_ARG_COUNT(1);
			DSR(arguments[0]);
			break;
		default:
			NOT_IMPLEMENTED();
			break;
		}

		reset_sequence();
	}

	LibK::vector<int> Terminal::parse_csi_arguments()
	{
		LibK::vector<int> args;

		// skipping "\e["
		const char *string = m_current_escape_sequence.data() + 2;
		while (isdigit(*string) || *string == ';')
		{
			int arg = strtol(string, const_cast<char **>(&string), 10);
			args.push_back(arg);

			if (*string == ';')
				string++;
		}

		return args;
	}

	void Terminal::unecho()
	{
		m_console->put_char_at(m_cursor_row, m_cursor_column, ' ');
		if (m_cursor_column == 0)
		{
			if (m_cursor_row == 0)
				return;

			m_cursor_column = m_console->get_width() - 1;
			m_cursor_row--;
		}
		else
		{
			m_cursor_column--;
		}

		m_console->set_cursor_at(m_cursor_row, m_cursor_column);
		m_console->put_char_at(m_cursor_row, m_cursor_column, ' ');
	}

	LibK::string Terminal::parse_key_event(key_event_t event)
	{
		LibK::string result;

		if (!(event.modifiers & Pressed))
			return result;

		if (!event.code_point)
			return result;

		// TODO: handle special keycodes and combinations like Ctrl^C

		result.push_back(event.code_point);
		return result;
	}

	void Terminal::advance()
	{
		m_cursor_column++;
		if (m_cursor_column == m_console->get_width())
		{
			CR();
			LF();
		}
	}

	void Terminal::BEL()
	{
		NOT_IMPLEMENTED();
	}

	void Terminal::BS()
	{
		if (m_cursor_column > 0)
		{
			m_cursor_column--;
			m_console->set_cursor_at(m_cursor_row, m_cursor_column);
		}
	}

	void Terminal::CR()
	{
		m_cursor_column = 0;
	}

	void Terminal::HT()
	{
		NOT_IMPLEMENTED();
	}

	void Terminal::LF()
	{
		m_cursor_row++;

		if (m_cursor_row == m_console->get_height())
		{
			m_cursor_row--;
			m_console->scroll_up();
			m_console->clear(m_console->get_height() - 1, 0, m_console->get_height() - 1, m_console->get_width() - 1);
		}
	}

	void Terminal::FF()
	{
		NOT_IMPLEMENTED();
	}

	void Terminal::ESC()
	{
		m_sequence_state = SequenceState::FeSequence;
		m_csi_state = CSIState::CSISequenceParams;
	}

	void Terminal::RI()
	{
		if (m_cursor_row == 0)
			m_console->scroll_down();
		else
		{
			m_cursor_row--;
			m_console->set_cursor_at(m_cursor_row, m_cursor_column);
		}
	}

	void Terminal::SS2()
	{
		NOT_IMPLEMENTED();
	}

	void Terminal::SS3()
	{
		NOT_IMPLEMENTED();
	}

	void Terminal::DCS()
	{
		NOT_IMPLEMENTED();
	}

	void Terminal::CSI()
	{
		m_sequence_state = SequenceState::CSISequence;
	}

	void Terminal::ST()
	{
		NOT_IMPLEMENTED();
	}

	void Terminal::OSC()
	{
		NOT_IMPLEMENTED();
	}

	void Terminal::SOS()
	{
		NOT_IMPLEMENTED();
	}

	void Terminal::PM()
	{
		NOT_IMPLEMENTED();
	}

	void Terminal::APC()
	{
		NOT_IMPLEMENTED();
	}

	void Terminal::CUU(int n)
	{
		(void)n;
		NOT_IMPLEMENTED();
	}

	void Terminal::CUD(int n)
	{
		(void)n;
		NOT_IMPLEMENTED();
	}

	void Terminal::CUF(int n)
	{
		(void)n;
		NOT_IMPLEMENTED();
	}

	void Terminal::CUB(int n)
	{
		(void)n;
		NOT_IMPLEMENTED();
	}

	void Terminal::CNL(int n)
	{
		(void)n;
		NOT_IMPLEMENTED();
	}

	void Terminal::CPL(int n)
	{
		(void)n;
		NOT_IMPLEMENTED();
	}

	void Terminal::CHA(int n)
	{
		(void)n;
		NOT_IMPLEMENTED();
	}

	void Terminal::CUP(int row, int column)
	{
		// TODO: Move 1-to-0 indexing switch from this layer to the console layer
		if (row > 0)
			row--;

		if (column > 0)
			column--;

		m_cursor_row = row;
		m_cursor_column = column;
		m_console->set_cursor_at(m_cursor_row, m_cursor_column);
	}

	void Terminal::ED(int type)
	{
		if (type != 2)
		{
			NOT_IMPLEMENTED();
			return;
		}

		m_console->clear();
	}

	void Terminal::EL(int type)
	{
		if (type < 0 || type > 2)
			return;

		size_t from_column = type == 0 ? m_cursor_column : 0;
		size_t to_column = type == 1 ? m_cursor_column : m_console->get_width() - 1;

		m_console->clear(m_cursor_row, from_column, m_cursor_row, to_column);
	}

	void Terminal::SU(int n)
	{
		while(n--)
			m_console->scroll_up();
	}

	void Terminal::SD(int n)
	{
		// TODO: More elaborate scroll functions
		while (n--)
			m_console->scroll_down();
	}

	void Terminal::VPA(int row)
	{
		// TODO: Move 1-to-0 indexing switch from this layer to the console layer
		if (row > 0)
			row--;

		m_cursor_row = row;
		m_console->set_cursor_at(m_cursor_row, m_cursor_column);
	}

	void Terminal::HVP(int row, int column)
	{
		(void)row;
		(void)column;
		NOT_IMPLEMENTED();
	}

	void Terminal::SGR(int type)
	{
		if (type == 0)
		{
			// TODO: Implement more attributes
			m_console->set_fg_color(ANSIColor::BrightWhite);
			m_console->set_bg_color(ANSIColor::Black);
			return;
		}

		if (type >= 30 && type < 38)
		{
			m_console->set_fg_color((ANSIColor)(type - 30));
			return;
		}

		if (type >= 40 && type < 48)
		{
			m_console->set_bg_color((ANSIColor)(type - 40));
			return;
		}

		if (type == 39)
		{
			m_console->set_fg_color(ANSIColor::BrightWhite);
			return;
		}

		if (type == 49)
		{
			m_console->set_bg_color(ANSIColor::Black);
			return;
		}

		if (type >= 90 && type < 98)
		{
			m_console->set_fg_color((ANSIColor)((int)ANSIColor::BrightBlack + type - 90));
			return;
		}

		if (type >= 100 && type < 108)
		{
			m_console->set_bg_color((ANSIColor)((int)ANSIColor::BrightBlack + type - 90));
			return;
		}

		NOT_IMPLEMENTED();
	}

	void Terminal::AUX(int type)
	{
		(void)type;
		NOT_IMPLEMENTED();
	}

	void Terminal::DSR(int type)
	{
		(void)type;
		NOT_IMPLEMENTED();
	}

} // namespace Kernel
