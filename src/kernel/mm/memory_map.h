#ifndef _KERNEL_MEMORY_H
#define _KERNEL_MEMORY_H

#include <stdint.h>

#define MEMMAP_USABLE 0
#define MEMMAP_RESERVED 1
#define MEMMAP_ACPI_RECLAIMABLE 2
#define MEMMAP_ACPI_NVS 3
#define MEMMAP_BAD_MEMORY 4
#define MEMMAP_BOOTLOADER_RECLAIMABLE 5
#define MEMMAP_EXECUTABLE_AND_MODULES 6
#define MEMMAP_FRAMEBUFFER 7
#define MEMMAP_RESERVED_MAPPED 8

#define ALIGN_DOWN(addr, align) ((addr) & ~((align) - 1))
#define ALIGN_UP(addr, align) (((addr) + (align) - 1) & ~((align) - 1))

#define MAX_MEMMAP_ENTRIES 256

typedef struct {
  uint64_t base;
  uint64_t length;
  uint64_t type;
} memory_map_entry_t;

typedef struct {
  memory_map_entry_t entries[MAX_MEMMAP_ENTRIES];
  uint64_t entry_count;
} memory_map_t;

void memmap_init(memory_map_t *map, memory_map_entry_t *entries,
                 uint64_t entry_count);

#endif // _KERNEL_MEMORY_H
