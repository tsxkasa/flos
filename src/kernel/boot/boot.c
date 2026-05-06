#include <boot/boot.h>
#include <cpu/halt.h>
#include <limine.h>
#include <mm/mm_types.h>
#include <stdint.h>

__attribute__((used, section(".limine_requests"))) static volatile uint64_t
    limine_base_revision[] = LIMINE_BASE_REVISION(6);

__attribute__((
    used,
    section(
        ".limine_requests"))) static volatile struct limine_framebuffer_request
    framebuffer_request = {.id = LIMINE_FRAMEBUFFER_REQUEST_ID, .revision = 0};

__attribute__((
    used,
    section(".limine_requests"))) static volatile struct limine_memmap_request
    memmap_request = {.id = LIMINE_MEMMAP_REQUEST_ID, .revision = 0};

__attribute__((
    used,
    section(".limine_requests"))) static volatile struct limine_hhdm_request
    hhdm_request = {.id = LIMINE_HHDM_REQUEST_ID, .revision = 0};

__attribute__((
    used,
    section(".limine_requests"))) static volatile struct limine_rsdp_request
    rsdp_request = {.id = LIMINE_RSDP_REQUEST_ID, .revision = 0};

__attribute__((used, section(".limine_requests"))) static volatile struct
    limine_executable_address_request kernel_addr_request = {
        .id = LIMINE_EXECUTABLE_ADDRESS_REQUEST_ID, .revision = 0};

__attribute__((used,
               section(".limine_requests_start"))) static volatile uint64_t
    limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end"))) static volatile uint64_t
    limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

static framebuffer_t kernel_fb;
static memory_map_t kernel_mmap;
static uint64_t kernel_hhdm_offset;
static kernel_addr_t executable_code;
static void *kernel_rsdp;

static void init_fb(void) {
  if (!framebuffer_request.response ||
      framebuffer_request.response->framebuffer_count < 1)
    hcf();

  struct limine_framebuffer *lfb =
      framebuffer_request.response->framebuffers[0];

  fb_init(&kernel_fb, (volatile uint32_t *)lfb->address, lfb->width,
          lfb->height, lfb->pitch);
}

static void init_memmap(void) {
  if (!memmap_request.response || memmap_request.response->entry_count < 1)
    hcf();

  memory_map_entry_t temp_entries[MAX_MEMMAP_ENTRIES];

  uint64_t count = memmap_request.response->entry_count;

  if (memmap_request.response->entry_count > MAX_MEMMAP_ENTRIES) {
    hcf();
  }

  for (uint64_t i = 0; i < count; i++) {
    struct limine_memmap_entry *src = memmap_request.response->entries[i];

    temp_entries[i].base = src->base;
    temp_entries[i].length = src->length;
    temp_entries[i].type = src->type;
  }

  memmap_init(&kernel_mmap, temp_entries, count);
}

static void init_hhdm(void) {
  if (!hhdm_request.response)
    hcf();

  kernel_hhdm_offset = hhdm_request.response->offset;
}

static void init_kernel_address(void) {
  if (!kernel_addr_request.response)
    hcf();

  kernel_addr_t a = {.phys = kernel_addr_request.response->physical_base,
                     .virt = kernel_addr_request.response->virtual_base};

  executable_code = a;
}

static void init_rsdp(void) {
  if (!rsdp_request.response)
    hcf();

  kernel_rsdp = rsdp_request.response->address;
}

void boot_init(void) {
  if (!LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision))
    hcf();

  init_fb();
  init_memmap();
  init_hhdm();
  init_kernel_address();
  init_rsdp();
}

framebuffer_t *boot_get_framebuffer(void) { return &kernel_fb; }

memory_map_t *boot_get_memmap(void) { return &kernel_mmap; }

uint64_t boot_get_hhdm_offset(void) { return kernel_hhdm_offset; }

kernel_addr_t boot_get_executable_addr(void) { return executable_code; }

void *boot_get_rsdp(void) { return kernel_rsdp; }
