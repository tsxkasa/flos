#include <boot/boot.h>
#include <cpu/halt.h>
#include <kernel/bitmap.h>
#include <kernel/printk.h>
#include <kernel/string.h>
#include <limits.h>
#include <mm/memory.h>
#include <mm/memory_map.h>
#include <mm/pmm.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

struct bitmap_pmm pmm_map;

static uint64_t cached_index = 0;

void init_bitmap_pmm() {
  memory_map_t *map = boot_get_memmap();

  uint64_t highest_addr = 0;
  for (int i = 0; i < (int)map->entry_count; i++) {
    memory_map_entry_t *e = &map->entries[i];
    if (e->type != MEMMAP_USABLE)
      continue;

    uint64_t top = e->base + e->length;
    if (top > highest_addr)
      highest_addr = top;
  }

  pmm_map.max_addressable = highest_addr;
  pmm_map.total_pages = BYTES_TO_PAGES(highest_addr);

  uint64_t bitmap_size = ALIGN_UP((pmm_map.total_pages + 7) / 8, PAGE_SIZE);
  pmm_map.bitmap.size = bitmap_size;
  pmm_map.bitmap.map = NULL;

  uintptr_t bitmap_phys_addr =
      ULONG_MAX; // no way physical address reaches this point lol
  for (int i = 0; i < (int)map->entry_count; i++) {
    memory_map_entry_t *e = &map->entries[i];
    if (e->type == MEMMAP_USABLE && e->length >= pmm_map.bitmap.size) {
      bitmap_phys_addr = e->base;
      break;
    }
  }

  if (bitmap_phys_addr == ULONG_MAX) {
    printk(LOG_ERR "pmm: no usable region large enough to hold bitmap");
    hcf();
  }

  pmm_map.bitmap.map = (uint8_t *)phys_to_higher_half_data(bitmap_phys_addr);

  memset((void *)pmm_map.bitmap.map, 0xFF, pmm_map.bitmap.size);

  for (int i = 0; i < (int)map->entry_count; i++) {
    memory_map_entry_t *e = &map->entries[i];
    if (e->type != MEMMAP_USABLE)
      continue;

    uint64_t start = ALIGN_UP(e->base, PAGE_SIZE);
    uint64_t end = ALIGN_DOWN(e->base + e->length, PAGE_SIZE);

    for (uint64_t addr = start; addr < end; addr += PAGE_SIZE) {
      if (addr >= bitmap_phys_addr &&
          addr < bitmap_phys_addr + pmm_map.bitmap.size) {
        continue;
      }
      pmm_free_page(addr);
    }
  }

  bitmap_set_bit(&pmm_map.bitmap, 0);

  for (int i = 0; i < map->entry_count; i++) {
    memory_map_entry_t *e = &map->entries[i];
    // Convert base and length to MB for human readability
    uint64_t base_mb = e->base / (1024 * 1024);
    uint64_t len_mb = e->length / (1024 * 1024);
  }

  printk(LOG_INFO "Bitmap PMM initialized.\n");
}

uintptr_t pmm_alloc_page() {
  for (uint64_t i = cached_index; i < pmm_map.total_pages; i++) {
    if (!bitmap_check_bit(&pmm_map.bitmap, i)) {
      bitmap_set_bit(&pmm_map.bitmap, i);
      cached_index = i + 1; // Next time, start searching at i + 1
      return (uintptr_t)(i * PAGE_SIZE);
    }
  }

  // if hit end, wrap around to see if any before cached is free
  for (uint64_t i = 0; i < cached_index; i++) {
    if (!bitmap_check_bit(&pmm_map.bitmap, i)) {
      bitmap_set_bit(&pmm_map.bitmap, i);
      cached_index = i + 1;
      return (uintptr_t)(i * PAGE_SIZE);
    }
  }

  printk(LOG_ERR "pmm: OUT OF MEMORY!\n");
  return 0;
}

void pmm_free_page(uintptr_t phys_addr) {
  uint64_t index = phys_addr / PAGE_SIZE;
  if (index >= pmm_map.total_pages)
    return;

  bitmap_unset_bit(&pmm_map.bitmap, index);

  if (index < cached_index) {
    cached_index = index;
  }
}
