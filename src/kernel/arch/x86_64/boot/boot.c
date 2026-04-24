#include "mm/memory.h"
#include <boot/boot.h>
#include <cpu/halt.h>
#include <limine.h>
#include <stdint.h>

__attribute__((used, section(".limine_requests"))) static volatile uint64_t
    limine_base_revision[] = LIMINE_BASE_REVISION(6);

__attribute__((
    used,
    section(
        ".limine_requests"))) static volatile struct limine_framebuffer_request
    framebuffer_request = {.id = LIMINE_FRAMEBUFFER_REQUEST_ID, .revision = 0};

__attribute__((
    used,
    section(".limine_requests"))) static volatile struct limine_memmap_request
    memmap_request = {.id = LIMINE_MEMMAP_REQUEST_ID, .revision = 0};

__attribute__((used,
               section(".limine_requests_start"))) static volatile uint64_t
    limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end"))) static volatile uint64_t
    limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

static framebuffer_t kernel_fb;
static memory_map_t kernel_mmap;

void boot_init(void) {
  if (!LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision))
    hcf();
  if (!framebuffer_request.response ||
      framebuffer_request.response->framebuffer_count < 1)
    hcf();
  if (!memmap_request.response || memmap_request.response->entry_count < 1)
    hcf();

  if (memmap_request.response->entry_count > MAX_MEMMAP_ENTRIES) {
    hcf();
  }

  struct limine_framebuffer *lfb =
      framebuffer_request.response->framebuffers[0];

  memory_map_entry_t temp_entries[MAX_MEMMAP_ENTRIES];

  uint64_t count = memmap_request.response->entry_count;
  if (count > 256)
    count = 256;

  for (uint64_t i = 0; i < count; i++) {
    struct limine_memmap_entry *src = memmap_request.response->entries[i];

    temp_entries[i].base = src->base;
    temp_entries[i].length = src->length;
    temp_entries[i].type = src->type;
  }

  memmap_init(&kernel_mmap, temp_entries, count);

  // Translate once. The rest of the kernel never touches
  // limine_framebuffer.
  fb_init(&kernel_fb, (volatile uint32_t *)lfb->address, lfb->width,
          lfb->height, lfb->pitch);
}

framebuffer_t *boot_get_framebuffer(void) { return &kernel_fb; }

memory_map_t *boot_get_memmap(void) { return &kernel_mmap; }
