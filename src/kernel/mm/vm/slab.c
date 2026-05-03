#include <kernel/string.h>
#include <mm/address.h>
#include <mm/mm_types.h>
#include <mm/pmm/pmm.h>
#include <mm/vm/slab.h>

#define MAX_CACHES 64
static struct kmem_cache cache_pool[MAX_CACHES];
static int cache_pool_idx = 0;

static inline void *get_fp(void *obj) { return *(void **)obj; }
static inline void set_fp(void *obj, void *fp) { *(void **)obj = fp; }

static struct slab_t *alloc_new_slab(struct kmem_cache *s) {
  uintptr_t phys = pmm_alloc_page();
  if (!phys)
    return NULL;

  struct slab_t *slab = (struct slab_t *)phys_to_higher_half_data(phys);

  size_t hdr = SLAB_ALIGN(sizeof(struct slab_t), s->size);
  size_t n = (PAGE_SIZE - hdr) / s->size;
  char *start = (char *)slab + hdr;

  for (size_t i = 0; i < n - 1; i++)
    set_fp(start + i * s->size, start + (i + 1) * s->size);
  set_fp(start + (n - 1) * s->size, NULL);

  slab->freelist = start;
  slab->next = NULL;
  slab->inuse = 0;
  slab->objects = (uint16_t)n;

  s->obj_per_slab = n;
  return slab;
}

static void *__refill(struct kmem_cache *s) {
  struct slab_t *slab = s->partial;

  if (!slab) {
    slab = alloc_new_slab(s);
    if (!slab)
      return NULL;
  } else
    s->partial = slab->next;

  void *obj = slab->freelist;
  s->cpu.freelist = get_fp(obj);
  s->cpu.slab = slab;

  slab->freelist = NULL;
  slab->inuse++;
  return obj;
}

struct kmem_cache *kmem_cache_create(size_t obj_size) {
  if (obj_size < sizeof(void *))
    obj_size = sizeof(void *);

  struct kmem_cache *s = &cache_pool[cache_pool_idx++];
  memset(s, 0, sizeof(*s));
  if (!s)
    return NULL;

  s->obj_size = obj_size;
  s->size = SLAB_ALIGN(obj_size, sizeof(void *));

  size_t hdr = SLAB_ALIGN(sizeof(struct slab_t), sizeof(void *));
  s->obj_per_slab = (PAGE_SIZE - hdr) / s->size;

  s->partial = NULL;
  s->cpu.freelist = NULL;
  s->cpu.slab = NULL;
  return s;
}

void *kmem_cache_alloc(struct kmem_cache *s) {
  void *obj = s->cpu.freelist;
  if (__builtin_expect(obj != NULL, 1)) {
    s->cpu.freelist = get_fp(obj);
    s->cpu.slab->inuse++;
    return obj;
  }
  return __refill(s);
}

void kmem_cache_free(struct kmem_cache *s, void *obj) {
  struct slab_t *slab =
      (struct slab_t *)((uintptr_t)obj & ~(uintptr_t)(PAGE_SIZE - 1));

  set_fp(obj, s->cpu.freelist);
  s->cpu.freelist = obj;
  slab->inuse--;

  if (slab != s->cpu.slab && slab->inuse < slab->objects) {
    slab->next = s->partial;
    s->partial = slab;
  }
}
