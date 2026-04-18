#ifndef FONTS_H
#define FONTS_H

#include <drivers/video/framebuffer.h>
#include <stdint.h>

#define PSF_FONT_MAGIC 0x864ab572

extern char _binary_assets_font_psf_start[];
extern char _binary_assets_font_psf_end[];

/*
    PSF2
*/
typedef struct {
  uint32_t magic;         /* magic bytes to identify PSF */
  uint32_t version;       /* zero */
  uint32_t headersize;    /* offset of bitmaps in file, 32 */
  uint32_t flags;         /* 0 if there's no unicode table */
  uint32_t numglyph;      /* number of glyphs */
  uint32_t bytesperglyph; /* size of each glyph */
  uint32_t height;        /* height in pixels */
  uint32_t width;         /* width in pixels */
} __attribute__((packed)) psf2_header_t;

typedef struct {
  psf2_header_t *header;
  uint8_t *glyphs; /* pointer to glyph bitmap data */
} psf_font_t;

/* Console state: tracks cursor position in character cells */
typedef struct {
  framebuffer_t *fb;
  psf_font_t font;
  uint32_t cx, cy; /* cursor col, row (in chars) */
  uint32_t fg, bg; /* foreground/background ARGB */
} console_t;

void psf_load(psf_font_t *font);
void console_init(console_t *con, framebuffer_t *fb, uint32_t fg, uint32_t bg);
void console_putchar(console_t *con, char c);
void console_puts(console_t *con, const char *s);

#endif // FONTS_H
