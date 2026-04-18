#ifndef BOOT_H
#define BOOT_H

#include <kernel/framebuffer.h>

// Populates kernel abstractions from bootloader responses.
// Called once at the very top of kmain, never again.
void boot_init(void);

// Returns the framebuffer populated by boot_init.
framebuffer_t *boot_get_framebuffer(void);

#endif // BOOT_H
