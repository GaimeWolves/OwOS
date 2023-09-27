#include <tty/Terminal.hpp>

#include <libk/kcstdlib.hpp>

#include <logging/logger.hpp>

#define NOT_IMPLEMENTED() unimplemented_control_sequence(__PRETTY_FUNCTION__)

#define CHECK_HAS_ARGS()    \
	if (arguments.empty()) \
	{                         \
		reset_sequence();     \
		break;                \
	}

#define CHECK_ARG_COUNT(n)    \
	if (arguments.size() < n) \
	{                         \
		reset_sequence();     \
		break;                \
	}

namespace Kernel
{
	static void unimplemented_control_sequence(const char *name)
	{
		log("Terminal", "Unimplemented control sequence: %s\n", name);
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
	}

	void Terminal::reset_sequence()
	{
		m_current_escape_sequence = "";
		m_sequence_state = SequenceState::None;
	}

	void Terminal::read_fe_sequence(char ch)
	{
		switch (ch)
		{
		case 'N':
			CSI();
			break;
		case 'O':
			SS3();
			break;
		case 'P':
			DCS();
			break;
		case '[':
			CSI();
			break;
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
			reset_sequence();
			break;
		}
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
			CHECK_HAS_ARGS()
			CUU(arguments[0]);
			break;
		case 'B':
			CHECK_HAS_ARGS()
			CUD(arguments[0]);
			break;
		case 'C':
			CHECK_HAS_ARGS()
			CUF(arguments[0]);
			break;
		case 'D':
			CHECK_HAS_ARGS()
			CUB(arguments[0]);
			break;
		case 'E':
			CHECK_HAS_ARGS()
			CNL(arguments[0]);
			break;
		case 'F':
			CHECK_HAS_ARGS()
			CPL(arguments[0]);
			break;
		case 'G':
			CHECK_HAS_ARGS()
			CHA(arguments[0]);
			break;
		case 'H':
			CHECK_ARG_COUNT(2)
			CUP(arguments[0], arguments[1]);
			break;
		case 'J':
			CHECK_HAS_ARGS()
			ED(arguments[0]);
			break;
		case 'K':
			CHECK_HAS_ARGS()
			EL(arguments[0]);
			break;
		case 'S':
			CHECK_HAS_ARGS()
			SU(arguments[0]);
			break;
		case 'T':
			CHECK_HAS_ARGS()
			SD(arguments[0]);
			break;
		case 'f':
			CHECK_ARG_COUNT(2)
			HVP(arguments[0], arguments[1]);
			break;
		case 'm':
			CHECK_HAS_ARGS()
			SGR(arguments[0]);
			break;
		case 'i':
			CHECK_HAS_ARGS()
			AUX(arguments[0]);
			break;
		case 'n':
			CHECK_HAS_ARGS()
			DSR(arguments[0]);
			break;
		default:
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
		NOT_IMPLEMENTED();
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
		(void)type;
		NOT_IMPLEMENTED();
	}

	void Terminal::SU(int n)
	{
		(void)n;
		NOT_IMPLEMENTED();
	}

	void Terminal::SD(int n)
	{
		(void)n;
		NOT_IMPLEMENTED();
	}

	void Terminal::HVP(int row, int column)
	{
		(void)row;
		(void)column;
		NOT_IMPLEMENTED();
	}

	void Terminal::SGR(int type)
	{
		if (type < 30 || type == 38 || type > 49 || type == 48)
		{
			NOT_IMPLEMENTED();
			return;
		}

		if (type >= 30 && type < 38)
		{
			m_console->set_fg_color((ANSIColor)(type - 30));
			return;
		}

		if (type >= 40 && type < 48)
		{
			m_console->set_bg_color((ANSIColor)(type - 30));
			return;
		}

		if (type == 39) {
			m_console->set_fg_color(ANSIColor::BrightWhite);
			return;
		}

		if (type == 49) {
			m_console->set_bg_color(ANSIColor::Black);
			return;
		}
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
