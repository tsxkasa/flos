#include <drivers/video/framebuffer.h>

void fb_init(framebuffer_t *fb, volatile uint32_t *addr, uint64_t width,
             uint64_t height, uint64_t pitch) {
  fb->addr = addr;
  fb->width = width;
  fb->height = height;
  fb->pitch = pitch;
}

/* pitch is in bytes; divide by 4 for uint32_t stride */
void fb_putpixel(framebuffer_t *fb, uint64_t x, uint64_t y, uint32_t color) {
  fb->addr[y * (fb->pitch / 4) + x] = color;
}

void fb_fill_rect(framebuffer_t *fb, uint64_t x, uint64_t y, uint64_t w,
                  uint64_t h, uint32_t color) {
  for (uint64_t row = y; row < y + h; row++)
    for (uint64_t col = x; col < x + w; col++)
      fb_putpixel(fb, col, row, color);
}

void fb_clear(framebuffer_t *fb, uint32_t color) {
  fb_fill_rect(fb, 0, 0, fb->width, fb->height, color);
}
