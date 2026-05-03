#include <kernel/printk.h>
#include <kernel/stdlib.h>
#include <mm/vm/slab.h>

#define KMALLOC_MIN_SHIFT 3
#define KMALLOC_MAX_SHIFT 12

struct kmalloc_hdr {
  struct kmem_cache *cache;
} __attribute__((aligned(sizeof(void *))));

static struct kmem_cache *kmalloc_caches[KMALLOC_MAX_SHIFT + 1];

static inline size_t ceil_log2(size_t x) {
  size_t r = 0;
  x--;
  while (x > 0) {
    x >>= 1;
    r++;
  }
  return r;
}

void init_kmalloc(void) {
  for (int i = KMALLOC_MIN_SHIFT; i <= KMALLOC_MAX_SHIFT; i++) {
    size_t size = 1 << i;

    kmalloc_caches[i] = kmem_cache_create(size);
  }

  printk(LOG_INFO "kmalloc initialized\n");
}

void *kmalloc(int size) {
  if (size == 0)
    return NULL;

  size_t total = size + sizeof(struct kmalloc_hdr);

  size_t shift = ceil_log2(total);
  if (shift < KMALLOC_MIN_SHIFT)
    shift = KMALLOC_MIN_SHIFT;

  if (shift > KMALLOC_MAX_SHIFT) {
    return NULL;
  }

  struct kmem_cache *cache = kmalloc_caches[shift];
  struct kmalloc_hdr *hdr = kmem_cache_alloc(cache);

  if (!hdr)
    return NULL;

  hdr->cache = cache;

  return (void *)(hdr + 1);
}

void kfree(void *ptr) {
  if (!ptr)
    return;

  struct kmalloc_hdr *hdr = ((struct kmalloc_hdr *)ptr) - 1;

  kmem_cache_free(hdr->cache, hdr);
}
