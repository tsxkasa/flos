#include <drivers/video/fonts.h>

void psf_load(psf_font_t *font) {
  font->header = (psf2_header_t *)_binary_assets_font_psf_start;
  /* Glyph bitmaps start right after the header */
  font->glyphs =
      (uint8_t *)_binary_assets_font_psf_start + font->header->headersize;
}

/* Draw a single glyph at pixel position (px, py) */
static void psf_draw_glyph(console_t *con, unsigned char c, uint64_t px,
                           uint64_t py) {
  psf2_header_t *header = con->font.header;
  uint32_t bytes_per_line = (header->width + 7) / 8;

  if (c >= header->numglyph)
    c = 0;

  uint8_t *glyph = con->font.glyphs + (uint32_t)c * header->bytesperglyph;

  for (uint32_t row = 0; row < header->height; row++) {
    for (uint32_t col = 0; col < header->width; col++) {
      uint8_t byte = glyph[row * bytes_per_line + col / 8];
      uint32_t color = (byte & (0x80 >> (col % 8))) ? con->fg : con->bg;
      fb_putpixel(con->fb, px + col, py + row, color);
    }
  }
}

void console_init(console_t *con, framebuffer_t *fb, uint32_t fg, uint32_t bg) {
  con->fb = fb;
  con->cx = 0;
  con->cy = 0;
  con->fg = fg;
  con->bg = bg;
  psf_load(&con->font);
  fb_clear(fb, bg);
}

void console_putchar(console_t *con, char c) {
  psf2_header_t *header = con->font.header;
  uint32_t cols = con->fb->width / header->width;
  uint32_t rows = con->fb->height / header->height;

  if (c == '\n') {
    con->cx = 0;
    con->cy++;
  } else if (c == '\r') {
    con->cx = 0;
  } else if (c == '\b') {
    if (con->cx > 0)
      con->cx--;
    psf_draw_glyph(con, ' ', con->cx * header->width, con->cy * header->height);
  } else {
    psf_draw_glyph(con, (unsigned char)c, con->cx * header->width,
                   con->cy * header->height);
    con->cx++;
  }

  /* Wrap at right edge */
  if (con->cx >= cols) {
    con->cx = 0;
    con->cy++;
  }

  /* TODO: scrolling when cy >= rows — for now just reset */
  if (con->cy >= rows) {
    con->cy = 0;
    fb_clear(con->fb, con->bg);
  }
}

void console_puts(console_t *con, const char *s) {
  while (*s)
    console_putchar(con, *s++);
}
