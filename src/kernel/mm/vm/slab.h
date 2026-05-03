#ifndef _KERNEL_SLAB_H
#define _KERNEL_SLAB_H

#include <stddef.h>
#include <stdint.h>

struct kmem_cache;

struct slab_t {
  void *freelist;
  struct slab_t *next;

  struct kmem_cache *cache;

  uint16_t inuse;
  uint16_t objects;
};

struct kmem_cache_cpu {
  void *freelist;
  struct slab_t *slab;
};

struct kmem_cache {
  size_t size;
  size_t obj_size;
  size_t obj_per_slab;

  struct slab_t *partial;

  // TODO: SMP hooks
  struct kmem_cache_cpu cpu;
};

struct kmem_cache *kmem_cache_create(size_t obj_size);
void *kmem_cache_alloc(struct kmem_cache *s);
void kmem_cache_free(struct kmem_cache *s, void *obj);

#endif // _KERNEL_SLAB_H
