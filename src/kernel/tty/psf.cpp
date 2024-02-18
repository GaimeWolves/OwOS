#include <tty/psf.hpp>

#include <logging/logger.hpp>

extern "C"
{
	extern uintptr_t __font_start;
	extern uintptr_t __font_end;
}

namespace Kernel
{
	PSFFont PSFFont::m_embedded = PSFFont();

	PSFFont::PSFFont()
	{
		m_filesize = (uintptr_t)&__font_end - (uintptr_t)&__font_start;

		auto *magic = (uint8_t *)&__font_start;

		if (magic[0] == PSF1_MAGIC0 && magic[1] == PSF1_MAGIC1) {
			m_header_psf1 = (psf1_header_t *)magic;
			parse_psf1();
		} else if (magic[0] == PSF2_MAGIC0 && magic[1] == PSF2_MAGIC1 && magic[2] == PSF2_MAGIC2 && magic[3] == PSF2_MAGIC3) {
			m_header_psf2 = (psf2_header_t *)magic;
		} else {
			log("ERROR", "Invalid console font data");
		}
	}

	psf_glyph_t PSFFont::get_glyph(char ch) const
	{
		if (m_header_psf1) {
			uint8_t *bitmap = nullptr;

			if (m_mappings[(uint8_t)ch] != -1) {
				bitmap = m_glyphs + m_mappings[(uint8_t)ch] * m_header_psf1->charsize;
			}

			return psf_glyph_t {
			    .bitmap = bitmap,
			    .stride = 8,
			    .width = 8,
			    .height = m_header_psf1->charsize,
			};
		} else {
			uint8_t *bitmap = nullptr;

			if (m_mappings[(uint8_t)ch] != -1) {
				bitmap = m_glyphs + m_mappings[(uint8_t)ch] * m_header_psf2->charsize;
			}

			return psf_glyph_t {
			    .bitmap = bitmap,
			    .stride = LibK::round_up_to_power_of_two<uint8_t>(m_header_psf2->width, CHAR_BIT),
			    .width = (uint8_t)m_header_psf2->width,
			    .height = (uint8_t)m_header_psf2->height,
			};
		}
	}

	void PSFFont::parse_psf1()
	{
		m_glyphs = (uint8_t *)m_header_psf1 + sizeof(psf1_header_t);

		if (!(m_header_psf1->mode & PSF1_MODEHASTAB)) {
			for (int i = 0; i <= UINT8_MAX; i++) {
				m_mappings[i] = i;
			}
		}

		size_t count = m_header_psf1->mode & PSF1_MODE512 ? 512 : 256;
		size_t offset = count * m_header_psf1->charsize;
		auto *current_unicode = (uint16_t *)(m_glyphs + offset);

		int glyph = 0;
		bool in_sequences = false;

		while ((uintptr_t)current_unicode - (uintptr_t)m_header_psf1 < m_filesize)
		{
			if (*current_unicode == PSF1_SEPARATOR) {
				in_sequences = false;
				glyph++;
			}
			else if (*current_unicode == PSF1_STARTSEQ) {
				in_sequences = true;
			} else if (!in_sequences && *current_unicode <= UINT8_MAX) {
				m_mappings[*current_unicode] = glyph;
			}

			current_unicode++;
		}
	}

	size_t PSFFont::get_width() const
	{
		if (m_header_psf1)
			return 8;
		else if (m_header_psf2)
			return m_header_psf2->width;
		return 0;
	}

	size_t PSFFont::get_height() const
	{
		if (m_header_psf1)
			return m_header_psf1->charsize;
		else if (m_header_psf2)
			return m_header_psf2->height;
		return 0;
	}
}
