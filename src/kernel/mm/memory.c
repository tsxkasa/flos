#include <mm/memory.h>

void memmap_init(memory_map_t *map, memory_map_entry_t *entries,
                 uint64_t entry_count) {
  if (entry_count > 256)
    entry_count = 256;

  map->entry_count = entry_count;

  for (uint64_t i = 0; i < entry_count; i++) {
    map->entries[i] = entries[i];
  }
}
