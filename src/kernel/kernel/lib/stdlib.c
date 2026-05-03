#include <kernel/kassert.h>
#include <kernel/printk.h>
#include <kernel/stdlib.h>
#include <mm/address.h>
#include <mm/mm_types.h>
#include <mm/pmm/pmm.h>
#include <mm/vm/slab.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>

typedef enum { SLAB_BACKED, PAGE_BACKED } alloc_type_t;

typedef struct {
  alloc_type_t type;
  union {
    size_t size;
    size_t slab_idx;
  } __attribute__((packed));
} __attribute__((aligned(16))) alloc_header_t;

static size_t kmalloc_sizes[] = {8,   16,  24,  32,  40,  48,   56,   64,  80,
                                 96,  112, 128, 160, 192, 224,  256,  320, 384,
                                 448, 512, 640, 768, 896, 1024, 2048, 3072};

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

void *kmalloc(size_t size) {
  if (size == 0)
    return NULL;

  size_t total = ALIGN_UP(size + sizeof(alloc_header_t), 16);
  alloc_header_t *hdr;

  int idx = kmalloc_index(total);

  if (idx >= 0) {
    hdr = kmem_cache_alloc(kmalloc_caches[idx]);
    if (!hdr)
      return NULL;

    hdr->type = SLAB_BACKED;
    hdr->slab_idx = idx;
  } else {
    int pages = (total + PAGE_SIZE - 1) / PAGE_SIZE;

    uintptr_t phys = pmm_alloc_pages(pages);
    if (!phys)
      return NULL;

    hdr = (alloc_header_t *)phys_to_higher_half_data(phys);

    if (!hdr)
      return NULL;

    hdr->type = PAGE_BACKED;
    hdr->size = pages;
  }

  return (void *)(hdr + 1);
}

void kfree(void *ptr) {
  if (!ptr)
    return;

  alloc_header_t *hdr = ((alloc_header_t *)ptr) - 1;

  if (hdr->type == SLAB_BACKED) {
    kmem_cache_free(kmalloc_caches[hdr->slab_idx], hdr);

  } else if (hdr->type == PAGE_BACKED) {
    pmm_free_pages(higher_half_data_to_phys((uintptr_t)hdr), hdr->size);
  }
}
