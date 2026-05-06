#include <kernel/kassert.h>
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

/**
 * @brief allocates new slab on the provided cache
 * note: the slab header is stored at the beginning of the page
 *
 * - slab is guaranteed to be pointer aligned
 *
 * @param s   the cache to allocate physical pages on
 * @return    the slab with allocated cache pool
 */
static struct slab_t *alloc_new_slab(struct kmem_cache *s) {
  uintptr_t phys = pmm_alloc_page();
  if (!phys)
    return NULL;

  struct slab_t *slab = (struct slab_t *)phys_to_higher_half_data(phys);

  size_t hdr = ALIGN_UP(sizeof(struct slab_t), sizeof(void *));
  size_t n = (PAGE_SIZE - hdr) / s->size;
  kassert(n > 0);
  char *start = (char *)slab + hdr;

  for (size_t i = 0; i < n - 1; i++)
    set_fp(start + i * s->size, start + (i + 1) * s->size);
  set_fp(start + (n - 1) * s->size, NULL);

  slab->freelist = start;
  slab->next = NULL;
  slab->inuse = 0;
  slab->cache = s;
  slab->objects = (uint16_t)n;

  s->obj_per_slab = n;
  return slab;
}

/**
 * @brief refills the per-cpu cache from slab
 * if a partial slab exists, it will be removed from the partial list
 * if not, a new slab will be allocated
 *
 * @param s cache to refill slab
 */
static void *_refill(struct kmem_cache *s) {
  struct slab_t *slab = s->partial;
  if (!slab) {
    slab = alloc_new_slab(s);
    if (!slab)
      return NULL;
  } else {
    s->partial = slab->next;
    slab->next = NULL;
  }

  kassert(slab->freelist != NULL);

  void *obj = slab->freelist;
  s->cpu.freelist = get_fp(obj);
  s->cpu.slab = slab;
  // freelist is transferred to cpu cache, the slab no longer needs it
  slab->freelist = NULL;
  slab->inuse++;
  return obj;
}

/**
 * @brief creates a cache for fixed size object allocation
 *
 * @param obj_size    the size of the object the cache will hold
 * @return            the cache
 */
struct kmem_cache *kmem_cache_create(size_t obj_size) {
  if (obj_size < sizeof(void *))
    obj_size = sizeof(void *);

  struct kmem_cache *s = &cache_pool[cache_pool_idx++];
  memset(s, 0, sizeof(*s));

  s->obj_size = obj_size;
  s->size = ALIGN_UP(obj_size, sizeof(void *));

  size_t hdr = ALIGN_UP(sizeof(struct slab_t), sizeof(void *));
  s->obj_per_slab = (PAGE_SIZE - hdr) / s->size;

  s->partial = NULL;
  s->cpu.freelist = NULL;
  s->cpu.slab = NULL;
  return s;
}

/**
 * @brief allocates cache and mark it as used, if no cache is readily available,
 * refills the pool
 *
 * @param s the cache pool to allocate to
 * @return  returns pointer to the allocated object or NULL on failure
 */
void *kmem_cache_alloc(struct kmem_cache *s) {
  void *obj = s->cpu.freelist;
  if (__builtin_expect(obj != NULL, 1)) {
    s->cpu.freelist = get_fp(obj);
    s->cpu.slab->inuse++;
    return obj;
  }
  return _refill(s);
}

/**
 * @brief frees the object back to its slab
 * the slab is determined by aligning the objects address to the page boundary
 *
 * @param s   the cache to free object from
 * @param obj pointer to the object to free
 */
void kmem_cache_free(struct kmem_cache *s, void *obj) {
  struct slab_t *slab =
      (struct slab_t *)((uintptr_t)obj & ~(uintptr_t)(PAGE_SIZE - 1));

  slab->inuse--;

  if (slab == s->cpu.slab) {
    set_fp(obj, s->cpu.freelist);
    s->cpu.freelist = obj;
  } else {
    bool was_full = (slab->freelist == NULL);
    set_fp(obj, slab->freelist);
    slab->freelist = obj;

    if (was_full) {
      slab->next = s->partial;
      s->partial = slab;
    }
  }
}
