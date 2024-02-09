#include <tty/Terminal.hpp>

#include <libk/kcstdlib.hpp>

#include <logging/logger.hpp>

#define NOT_IMPLEMENTED() unimplemented_control_sequence(m_current_escape_sequence.c_str(), __PRETTY_FUNCTION__)

#define ENSURE_ARG_COUNT(n) arguments.resize(n, 0)

namespace Kernel
{
	static void unimplemented_control_sequence(const char *sequence, const char *name)
	{
		log("TERMINAL", "Unimplemented control sequence %s at %s", sequence, name);
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
			this->write_char(ch, m_cursor.row, m_cursor.column);
		}
		else
		{
			log("TERMINAL", "Ignoring %c 0x%x", ch, ch);
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
		case '7':
			DECSC();
			break;
		case '8':
			DECRC();
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

		bool is_private_mode = m_current_escape_sequence[2] == '?';

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
		case 'b':
			ENSURE_ARG_COUNT(1);
			REP(arguments[0]);
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
		case 'h':
			ENSURE_ARG_COUNT(1);
			if (is_private_mode)
				DECSET(arguments[0]);
			break;
		case 'i':
			ENSURE_ARG_COUNT(1);
			AUX(arguments[0]);
			break;
		case 'l':
			ENSURE_ARG_COUNT(1);
			if (is_private_mode)
				DECRST(arguments[0]);
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

		// check for private sequences (TODO: not very extensible)
		if (*string == '?')
			string++;

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
		m_console->write_char(m_cursor.row, m_cursor.column, ' ');
		if (m_cursor.column == 0)
		{
			if (m_cursor.row == 0)
				return;

			m_cursor.column = m_console->get_width() - 1;
			m_cursor.row--;
		}
		else
		{
			m_cursor.column--;
		}

		m_console->set_cursor_at(m_cursor.row, m_cursor.column);
		m_console->write_char(m_cursor.row, m_cursor.column, ' ');
	}

	LibK::string Terminal::parse_key_event(key_event_t event)
	{
		LibK::string result;

		if (!(event.modifiers & Pressed))
			return result;

		// TODO: handle special keycodes and combinations like Ctrl^C

		switch (event.key)
		{
		case Key_Up:
			result += "\e[A";
			return result;
		case Key_Down:
			result += "\e[B";
			return result;
		case Key_Right:
			result += "\e[C";
			return result;
		case Key_Left:
			result += "\e[D";
			return result;
		default:
			break;
		}

		if (!event.code_point)
			return result;

		result.push_back(event.code_point);
		return result;
	}

	void Terminal::write_char(char ch, size_t row, size_t column)
	{
		m_console->write_char(row, column, ch);
		get_buffer()[row * m_console->get_width() + column] = { ch, m_cursor.fg_color, m_cursor.bg_color };
		m_last_char = ch;
		advance();
	}

	void Terminal::switch_buffer(bool alternate, bool clear)
	{
		if (m_use_alternate_buffer == alternate)
			return;

		m_use_alternate_buffer = alternate;

		Console::cell_t *buffer = m_use_alternate_buffer ? m_alternate_buffer : m_main_buffer;

		if (!clear)
			m_console->write_region(0, 0, m_console->get_height() - 1, m_console->get_width() - 1, buffer);
		else
		{
			this->clear(0, 0, m_console->get_height() - 1, m_console->get_width() - 1);
		}
	}

	void Terminal::advance()
	{
		if (m_cursor.column < m_console->get_width() - 1)
		{
			set_cursor_column(m_cursor.column + 1);
			return;
		}

		CR();
		LF();
	}

	void Terminal::set_cursor_row(size_t row)
	{
		set_cursor(row, m_cursor.column);
	}

	void Terminal::set_cursor_column(size_t column)
	{
		set_cursor(m_cursor.row, column);
	}

	void Terminal::set_cursor(size_t row, size_t column)
	{
		m_cursor.row = row;
		m_cursor.column = column;
		m_console->set_cursor_at(row, column);
	}

	void Terminal::clear(size_t from_row, size_t from_column, size_t to_row, size_t to_column)
	{
		size_t from_index = from_row * m_console->get_width() + from_column;
		size_t to_index = to_row * m_console->get_width() + to_column;

		for (size_t i = from_index; i <= to_index; i++)
			get_buffer()[i] = Console::default_cell;
		m_console->clear(from_row, from_column, to_row, to_column);
	}

	void Terminal::scroll_up()
	{
		// TODO: Scrollback buffer
		m_console->scroll_up();
		m_console->clear(m_console->get_height() - 1, 0, m_console->get_height() - 1, m_console->get_width() - 1);

		memmove(get_buffer(), get_buffer() + m_console->get_width(), m_console->get_width() * (m_console->get_height() - 1) * sizeof(Console::cell_t));
		for (size_t i = m_console->get_width() * (m_console->get_height() - 1); i < m_console->get_width() * m_console->get_height(); i++)
			get_buffer()[i] = Console::default_cell;
	}

	void Terminal::scroll_down()
	{
		// TODO: Scrollback buffer
		m_console->scroll_down();
		m_console->clear(0, 0, 0, m_console->get_width() - 1);

		memmove(get_buffer() + m_console->get_width(), get_buffer(), m_console->get_width() * (m_console->get_height() - 1) * sizeof(Console::cell_t));
		for (size_t i = 0; i < m_console->get_width(); i++)
			get_buffer()[i] = Console::default_cell;
	}

	void Terminal::BEL()
	{
		NOT_IMPLEMENTED();
	}

	void Terminal::BS()
	{
		if (m_cursor.column > 0)
		{
			set_cursor_column(m_cursor.column - 1);
		}
	}

	void Terminal::CR()
	{
		set_cursor_column(0);
	}

	void Terminal::HT()
	{
		NOT_IMPLEMENTED();
	}

	void Terminal::LF()
	{
		if (m_cursor.row < m_console->get_height() - 1)
		{
			set_cursor_row(m_cursor.row + 1);
			return;
		}

		scroll_up();
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
		if (m_cursor.row == 0)
		{
			scroll_down();
		}
		else
		{
			set_cursor_row(m_cursor.row - 1);
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

	void Terminal::DECSC()
	{
		if (m_use_alternate_buffer)
			m_alternate_saved_cursor = m_cursor;
		else
			m_saved_cursor = m_cursor;
	}

	void Terminal::DECRC()
	{
		cursor_t cursor = m_use_alternate_buffer ? m_alternate_saved_cursor : m_saved_cursor;
		set_cursor(cursor.row, cursor.column);
	}

	void Terminal::CUU(int n)
	{
		size_t new_row = LibK::min(m_cursor.row, n);
		set_cursor(new_row, m_cursor.column);
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
		size_t new_column = LibK::min(LibK::max(n - 1, 0), m_console->get_width() - 1);
		set_cursor(m_cursor.row, new_column);
	}

	void Terminal::CUP(int row, int column)
	{
		// TODO: Move 1-to-0 indexing switch from this layer to the console layer
		if (row > 0)
			row--;

		if (column > 0)
			column--;

		set_cursor(row, column);
	}

	void Terminal::ED(int type)
	{
		if (type != 2)
		{
			NOT_IMPLEMENTED();
			return;
		}

		this->clear(0, 0, m_console->get_height() - 1, m_console->get_width() - 1);
	}

	void Terminal::EL(int type)
	{
		if (type < 0 || type > 2)
			return;

		size_t from_column = type == 0 ? m_cursor.column : 0;
		size_t to_column = type == 1 ? m_cursor.column : m_console->get_width() - 1;

		m_console->clear(m_cursor.row, from_column, m_cursor.row, to_column);
		for (size_t i = m_console->get_width() * m_cursor.row + from_column; i <= m_console->get_width() * m_cursor.row + to_column; i++)
			get_buffer()[i] = Console::default_cell;
	}

	void Terminal::SU(int n)
	{
		while(n--)
			scroll_up();
	}

	void Terminal::SD(int n)
	{
		// TODO: More elaborate scroll functions
		while (n--)
			scroll_down();
	}

	void Terminal::REP(int n)
	{
		if (n == 0)
			n = 1;

		while (n--)
			write_char(m_last_char, m_cursor.row, m_cursor.column);
	}

	void Terminal::VPA(int row)
	{
		// TODO: Move 1-to-0 indexing switch from this layer to the console layer
		if (row > 0)
			row--;

		set_cursor_row(row);
	}

	void Terminal::HVP(int row, int column)
	{
		(void)row;
		(void)column;
		NOT_IMPLEMENTED();
	}

	void Terminal::DECSET(int mode)
	{
		switch (mode)
		{
		case 1049:
			m_saved_cursor = m_cursor;
			switch_buffer(true, true);
			return;
		}

		NOT_IMPLEMENTED();
	}

	void Terminal::DECRST(int mode)
	{
		switch (mode)
		{
		case 1049:
			switch_buffer(false, false);
			set_cursor(m_saved_cursor.row, m_saved_cursor.column);
			return;
		}

		NOT_IMPLEMENTED();
	}

	void Terminal::SGR(int type)
	{
		if (type == 0)
		{
			// TODO: Implement more attributes
			m_cursor.fg_color = ANSIColorToRGBA(ANSIColor::BrightWhite);
			m_cursor.bg_color = ANSIColorToRGBA(ANSIColor::Black);
			m_console->set_fg_color(ANSIColor::BrightWhite);
			m_console->set_bg_color(ANSIColor::Black);
			return;
		}

		if (type >= 30 && type < 38)
		{
			m_cursor.fg_color = ANSIColorToRGBA((ANSIColor)(type - 30));
			m_console->set_fg_color((ANSIColor)(type - 30));
			return;
		}

		if (type >= 40 && type < 48)
		{
			m_cursor.bg_color = ANSIColorToRGBA((ANSIColor)(type - 40));
			m_console->set_bg_color((ANSIColor)(type - 40));
			return;
		}

		if (type == 39)
		{
			m_cursor.fg_color = ANSIColorToRGBA(ANSIColor::BrightWhite);
			m_console->set_fg_color(ANSIColor::BrightWhite);
			return;
		}

		if (type == 49)
		{
			m_cursor.bg_color = ANSIColorToRGBA(ANSIColor::Black);
			m_console->set_bg_color(ANSIColor::Black);
			return;
		}

		if (type >= 90 && type < 98)
		{
			m_cursor.fg_color = ANSIColorToRGBA((ANSIColor)((int)ANSIColor::BrightBlack + type - 90));
			m_console->set_fg_color((ANSIColor)((int)ANSIColor::BrightBlack + type - 90));
			return;
		}

		if (type >= 100 && type < 108)
		{
			m_cursor.bg_color = ANSIColorToRGBA((ANSIColor)((int)ANSIColor::BrightBlack + type - 90));
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
