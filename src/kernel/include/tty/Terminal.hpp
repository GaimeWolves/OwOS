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

	public:
		explicit Terminal(Console *console)
		    : m_console(console)
		{
		}

		void echo(char ch);
		void unecho();

		LibK::string parse_key_event(key_event_t event);

	private:
		void advance();

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
		void SS2();
		void SS3();
		void DCS();
		void CSI();
		void ST();
		void OSC();
		void SOS();
		void PM();
		void APC();

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
		void HVP(int row, int column);
		void SGR(int type);
		void AUX(int type);
		void DSR(int type);

		Console *m_console{nullptr};
		size_t m_cursor_row{1}, m_cursor_column{1};
		LibK::string m_current_escape_sequence{};

		SequenceState m_sequence_state{SequenceState::None};
		CSIState m_csi_state{CSIState::CSISequenceParams};
	};
}