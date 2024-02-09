#pragma once

#include <libk/kstring.hpp>

#include <tty/Console.hpp>
#include <devices/KeyboardDevice.hpp>

namespace Kernel
{
	class Terminal
	{
		enum class SequenceState
		{
			None,
			FeSequence,
			CSISequence,
			OSCSequence,
		};

		enum class CSIState
		{
			CSISequenceParams,
			CSISequenceIntermediate,
			CSISequenceFinal,
		};

		typedef struct __cursor_t
		{
			size_t row;
			size_t column;
			uint32_t fg_color;
			uint32_t bg_color;
		} cursor_t;

	public:
		explicit Terminal(Console *console)
		    : m_console(console)
		{
			size_t size = sizeof(Console::cell_t) * console->get_height() * console->get_width();
			m_main_buffer = static_cast<Console::cell_t *>(kmalloc(size, alignof(Console::cell_t)));
			m_alternate_buffer = static_cast<Console::cell_t *>(kmalloc(size, alignof(Console::cell_t)));

			memset(m_main_buffer, 0, size);
			memset(m_alternate_buffer, 0, size);
		}

		void echo(char ch);
		void unecho();

		LibK::string parse_key_event(key_event_t event);

	private:
		void advance();

		Console::cell_t *get_buffer() { return m_use_alternate_buffer ? m_alternate_buffer : m_main_buffer; }
		void write_char(char ch, size_t row, size_t column);

		void set_cursor_row(size_t row);
		void set_cursor_column(size_t column);
		void set_cursor(size_t row, size_t column);

		void clear(size_t from_row, size_t from_column, size_t to_row, size_t to_column);
		void scroll_up();
		void scroll_down();

		void switch_buffer(bool alternate, bool clear);

		void reset_sequence();
		void read_fe_sequence(char ch);
		void read_csi_sequence(char ch);
		LibK::vector<int> parse_csi_arguments();

		// C0 control codes
		void BEL();
		void BS();
		void CR();
		void HT();
		void LF();
		void FF();
		void ESC();

		// C1 control codes
		void RI();
		void SS2();
		void SS3();
		void DCS();
		void CSI();
		void ST();
		void OSC();
		void SOS();
		void PM();
		void APC();

		void DECSC();
		void DECRC();

		// CSI sequences
		void CUU(int n);
		void CUD(int n);
		void CUF(int n);
		void CUB(int n);
		void CNL(int n);
		void CPL(int n);
		void CHA(int n);
		void CUP(int row, int column);
		void ED(int type);
		void EL(int type);
		void SU(int n);
		void SD(int n);
		void REP(int n);
		void VPA(int row);
		void HVP(int row, int column);
		void DECSET(int mode);
		void DECRST(int mode);
		void SGR(int type);
		void AUX(int type);
		void DSR(int type);

		Console::cell_t *m_main_buffer{nullptr};
		cursor_t m_saved_cursor{};

		Console::cell_t *m_alternate_buffer{nullptr};
		cursor_t m_alternate_saved_cursor{};

		bool m_use_alternate_buffer{false};

		Console *m_console{nullptr};
		cursor_t m_cursor{};
		LibK::string m_current_escape_sequence{};

		char m_last_char{' '};

		SequenceState m_sequence_state{SequenceState::None};
		CSIState m_csi_state{CSIState::CSISequenceParams};
	};
}