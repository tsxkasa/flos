#include <kernel/printk.h>
#include <kernel/stdlib.h>
#include <mm/mm_types.h>
#include <mm/vm/slab.h>
#include <stddef.h>

static size_t kmalloc_sizes[] = {8,   16,  24,  32,  40,  48,   56,   64,  80,
                                 96,  112, 128, 160, 192, 224,  256,  320, 384,
                                 448, 512, 640, 768, 896, 1024, 2048, 4096};

#define KMALLOC_NUM_SIZES (sizeof(kmalloc_sizes) / sizeof(size_t))
static struct kmem_cache *kmalloc_caches[KMALLOC_NUM_SIZES];

void init_kmalloc(void) {
  for (size_t i = 0; i < KMALLOC_NUM_SIZES; i++) {
    kmalloc_caches[i] = kmem_cache_create(kmalloc_sizes[i]);
  }

  printk(LOG_INFO "kmalloc initialized\n");
}

static int kmalloc_index(size_t size) {
  int left = 0;
  int right = KMALLOC_NUM_SIZES - 1;
  int result = -1;

  while (left <= right) {
    int mid = (left + right) / 2;

    if (kmalloc_sizes[mid] >= size) {
      result = mid;
      right = mid - 1;
    } else {
      left = mid + 1;
    }
  }

  return result;
}

void *kmalloc(unsigned int size) {
  if (size == 0)
    return NULL;

  int idx = kmalloc_index(size);
  if (idx < 0) {
    return NULL;
  }

  return kmem_cache_alloc(kmalloc_caches[idx]);
}

void kfree(void *ptr) {
  if (!ptr)
    return;

  struct slab_t *slab = (struct slab_t *)((uintptr_t)ptr & ~(PAGE_SIZE - 1));

  kmem_cache_free(slab->cache, ptr);
}
