#include <boot/boot.h>
#include <drivers/tty/tty.h>
#include <drivers/video/font/psf_font.h>
#include <drivers/video/framebuffer/framebuffer.h>
#include <kernel/printk.h>
#include <kernel/string.h>
#include <stdint.h>

#define MAX_ROWS 512

static char tty_buffer[MAX_ROWS][256];
static uint32_t tty_row = 0;
static uint32_t view_row = 0;
static uint32_t cols = 0;
static uint32_t rows = 0;

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

static void tty_redraw(void) {
  fb_clear(term_fb, term_bg);
  psf2_header_t *h = term_font.header;

  for (uint32_t row = 0; row < rows; row++) {
    uint32_t buf_row = (view_row + row) % MAX_ROWS;
    char *line = tty_buffer[buf_row];

    for (uint32_t col = 0; col < cols; col++) {
      char c = line[col];
      if (!c)
        continue;
      draw_glyph(c, col * h->width, row * h->height);
    }
  }
}

void tty_init(void) {
  term_fb = boot_get_framebuffer();
  psf_load(&term_font);

  cols = term_fb->width / term_font.header->width;
  rows = term_fb->height / term_font.header->height;

  term_cx = 0;
  tty_row = 0;
  view_row = 0;

  memset(tty_buffer, 0, sizeof(tty_buffer));
  fb_clear(term_fb, term_bg);
}

void tty_scroll_down(void) {
  if (view_row + rows < tty_row) {
    view_row++;
    tty_redraw();
  }
}

void tty_scroll_up(void) {
  if (view_row > 0) {
    view_row--;
    tty_redraw();
  }
}

void tty_putchar(char c) {
  psf2_header_t *h = term_font.header;
  char *line = tty_buffer[tty_row % MAX_ROWS];

  if (c == '\n') {
    tty_row++;
    term_cx = 0;

    memset(tty_buffer[tty_row % MAX_ROWS], 0, cols);

    if (tty_row - view_row >= rows) {
      view_row++;
      tty_redraw();
    }

  } else if (c == '\r') {
    term_cx = 0;

  } else if (c == '\b') {
    if (term_cx > 0) {
      term_cx--;
      line[term_cx] = ' ';

      uint32_t vis_row = tty_row - view_row;
      if (vis_row < rows)
        draw_glyph(' ', term_cx * h->width, vis_row * h->height);
    }

  } else {
    if (term_cx < cols) {
      line[term_cx] = c;

      uint32_t vis_row = tty_row - view_row;
      if (vis_row < rows)
        draw_glyph(c, term_cx * h->width, vis_row * h->height);

      term_cx++;
    }
  }
}

void tty_write(const char *data, size_t size) {
  for (size_t i = 0; i < size; i++)
    tty_putchar(data[i]);
}

void tty_writestring(const char *data) {
  while (*data)
    tty_putchar(*data++);
}

void tty_clear(void) {
  memset(tty_buffer, 0, sizeof(tty_buffer));
  tty_row = 0;
  view_row = 0;
  term_cx = 0;
  fb_clear(term_fb, term_bg);
}

void tty_flush(void) {
  char buf[128];
  int n;

  while ((n = printk_read_buf(buf, sizeof(buf))) > 0) {
    tty_write(buf, n);
  }
}
