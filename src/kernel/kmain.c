#include <boot/boot.h>
#include <cpu/halt.h>
#include <drivers/video/fonts.h>
#include <drivers/video/framebuffer.h>
#include <limine.h>
#include <stdint.h>

// entry point
void kmain(void) {
  // asm volatile("int3"); // DEBUG
  boot_init();

  framebuffer_t *fb = boot_get_framebuffer();

  // volatile uint32_t *fb_ptr = fb->addr;
  // for (size_t y = 0; y < fb->height; y++) {
  //   for (size_t x = 0; x < fb->width; x++) {
  //     uint32_t nX = x * 255 / fb->width;
  //     uint32_t nY = y * 255 / fb->height;
  //     fb_ptr[y * (fb->pitch / 4) + x] = (nY << 8) | nX;
  //   }
  // }

  console_t con;
  console_init(&con, fb, 0xFFFFFFFF, 0x00000000);
  console_puts(&con, "hello iso");

  // We're done, just hang...
  hcf();
}
