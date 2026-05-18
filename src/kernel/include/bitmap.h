#ifndef _KERNEL_BITMAP_H
#define _KERNEL_BITMAP_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  uint64_t size;
  uint8_t *map;
} bitmap_t;

static inline void bitmap_set_bit(bitmap_t *bitmap, int bit) {
  bitmap->map[bit / 8] |= (1 << (bit % 8));
}

static inline void bitmap_unset_bit(bitmap_t *bitmap, int bit) {
  bitmap->map[bit / 8] &= ~(1 << (bit % 8));
}

static inline bool bitmap_check_bit(bitmap_t *bitmap, int bit) {
  return bitmap->map[bit / 8] & (1 << (bit % 8));
}

#endif // _KERNEL_BITMAP_H
