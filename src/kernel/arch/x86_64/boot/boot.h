#ifndef BOOT_H
#define BOOT_H

#include <drivers/video/framebuffer/framebuffer.h>
#include <mm/memory.h>
#include <stdint.h>

// Populates kernel abstractions from bootloader responses.
// Called once at the very top of kmain, never again.
void boot_init(void);

// Returns the framebuffer populated by boot_init.
framebuffer_t *boot_get_framebuffer(void);

memory_map_t *boot_get_memmap(void);

uint64_t boot_get_hhdm_offset(void);

#endif // BOOT_H
