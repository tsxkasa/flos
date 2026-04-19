#include <boot/boot.h>
#include <drivers/tty/tty.h>
#include <drivers/video/framebuffer.h>
#include <drivers/video/psf_font.h>
#include <stdint.h>

// Global Terminal State
static framebuffer_t *term_fb;
static psf_font_t term_font;
static uint32_t term_cx = 0;
static uint32_t term_cy = 0;
static uint32_t term_fg = 0xFFFFFFFF; // White
static uint32_t term_bg = 0x00000000; // Black

static void draw_glyph(unsigned char c, uint64_t px, uint64_t py) {
  psf2_header_t *header = term_font.header;
  uint32_t bytes_per_line = (header->width + 7) / 8;

  if (c >= header->numglyph)
    c = 0;
  uint8_t *glyph = term_font.glyphs + (uint32_t)c * header->bytesperglyph;

  for (uint32_t row = 0; row < header->height; row++) {
    for (uint32_t col = 0; col < header->width; col++) {
      uint8_t byte = glyph[row * bytes_per_line + col / 8];
      uint32_t color = (byte & (0x80 >> (col % 8))) ? term_fg : term_bg;
      fb_putpixel(term_fb, px + col, py + row, color);
    }
  }
}

void tty_init(void) {
  term_fb = boot_get_framebuffer();
  psf_load(&term_font);
  term_cx = 0;
  term_cy = 0;
  fb_clear(term_fb, term_bg);
}

void tty_putchar(char c) {
  psf2_header_t *header = term_font.header;
  uint32_t cols = term_fb->width / header->width;
  uint32_t rows = term_fb->height / header->height;

  if (c == '\n') {
    term_cx = 0;
    term_cy++;
  } else if (c == '\r') {
    term_cx = 0;
  } else if (c == '\b') {
    if (term_cx > 0)
      term_cx--;
    draw_glyph(' ', term_cx * header->width, term_cy * header->height);
  } else {
    draw_glyph((unsigned char)c, term_cx * header->width,
               term_cy * header->height);
    term_cx++;
  }

  // Wrap text
  if (term_cx >= cols) {
    term_cx = 0;
    term_cy++;
  }

  // TODO: scroll line
  if (term_cy >= rows) {
    term_cy = 0;
    fb_clear(term_fb, term_bg);
  }
}

void tty_write(const char *data, size_t size) {
  for (size_t i = 0; i < size; i++) {
    tty_putchar(data[i]);
  }
}

void tty_writestring(const char *data) {
  while (*data) {
    tty_putchar(*data++);
  }
}

void tty_clear(void) { fb_clear(term_fb, term_bg); }
