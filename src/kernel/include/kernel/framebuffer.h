#ifndef _KERNEL_FRAMEBUFFER_H
#define _KERNEL_FRAMEBUFFER_H
#include <stddef.h>
#include <stdint.h>

typedef struct {
  volatile uint32_t *addr;
  uint64_t width;
  uint64_t height;
  uint64_t pitch; /* bytes per row */
} framebuffer_t;

void fb_init(framebuffer_t *fb, volatile uint32_t *addr, uint64_t width,
             uint64_t height, uint64_t pitch);

void fb_putpixel(framebuffer_t *fb, uint64_t x, uint64_t y, uint32_t color);
void fb_fill_rect(framebuffer_t *fb, uint64_t x, uint64_t y, uint64_t w,
                  uint64_t h, uint32_t color);
void fb_clear(framebuffer_t *fb, uint32_t color);

#endif // _KERNEL_FRAMEBUFFER_H
