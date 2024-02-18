#pragma once

#include <stdint.h>
#include <stddef.h>

#include <common_attributes.h>

#define PSF1_MAGIC0     0x36
#define PSF1_MAGIC1     0x04

#define PSF1_MODE512    0x01
#define PSF1_MODEHASTAB 0x02
#define PSF1_MODEHASSEQ 0x04
#define PSF1_MAXMODE    0x05

#define PSF1_SEPARATOR  0xFFFF
#define PSF1_STARTSEQ   0xFFFE

#define PSF2_MAGIC0     0x72
#define PSF2_MAGIC1     0xb5
#define PSF2_MAGIC2     0x4a
#define PSF2_MAGIC3     0x86

/* bits used in flags */
#define PSF2_HAS_UNICODE_TABLE 0x01

/* max version recognized so far */
#define PSF2_MAXVERSION 0

/* UTF8 separators */
#define PSF2_SEPARATOR  0xFF
#define PSF2_STARTSEQ   0xFE

namespace Kernel
{
	typedef struct psf1_header_t
	{
		uint8_t magic[2];
		uint8_t mode;
		uint8_t charsize;
	} __packed psf1_header_t;

	typedef struct psf2_header_t {
		uint8_t magic[4];
		uint32_t version;
		uint32_t headersize;
		uint32_t flags;
		uint32_t length;
		uint32_t charsize;
		uint32_t height, width; /* charsize = height * ((width + 7) / 8) */
	} __packed psf2_header_t;

	typedef struct psf_glyph_t
	{
		uint8_t *bitmap;
		uint8_t stride;
		uint8_t width;
		uint8_t height;
	} psf_glyph_t;

	class PSFFont
	{
	public:
		static PSFFont &get_embedded() {
			return m_embedded;
		}

		PSFFont();

		[[nodiscard]] psf_glyph_t get_glyph(char ch) const;

		[[nodiscard]] size_t get_width() const;
		[[nodiscard]] size_t get_height() const;

	private:
		void parse_psf1();

		psf1_header_t *m_header_psf1{nullptr};
		psf2_header_t *m_header_psf2{nullptr};
		uint8_t *m_glyphs{nullptr};
		int m_mappings[UINT8_MAX + 1]{ -1 };
		size_t m_filesize{};

		static PSFFont m_embedded;
	};
}
