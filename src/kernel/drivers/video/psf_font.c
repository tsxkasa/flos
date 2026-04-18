#include <drivers/video/psf_font.h>

void psf_load(psf_font_t *font) {
  font->header = (psf2_header_t *)_binary_assets_font_psf_start;
  /* Glyph bitmaps start right after the header */
  font->glyphs =
      (uint8_t *)_binary_assets_font_psf_start + font->header->headersize;
}
