#include <boot/boot.h>
#include <cpu/halt.h>
#include <limine.h>

__attribute__((used, section(".limine_requests"))) static volatile uint64_t
    limine_base_revision[] = LIMINE_BASE_REVISION(6);

__attribute__((
    used,
    section(
        ".limine_requests"))) static volatile struct limine_framebuffer_request
    framebuffer_request = {.id = LIMINE_FRAMEBUFFER_REQUEST_ID, .revision = 0};

__attribute__((used,
               section(".limine_requests_start"))) static volatile uint64_t
    limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end"))) static volatile uint64_t
    limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

static framebuffer_t kernel_fb;

void boot_init(void) {
  if (!LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision))
    hcf();
  if (!framebuffer_request.response ||
      framebuffer_request.response->framebuffer_count < 1)
    hcf();

  struct limine_framebuffer *lfb =
      framebuffer_request.response->framebuffers[0];

  // Translate once. The rest of the kernel never touches limine_framebuffer.
  fb_init(&kernel_fb, (volatile uint32_t *)lfb->address, lfb->width,
          lfb->height, lfb->pitch);
}

framebuffer_t *boot_get_framebuffer(void) { return &kernel_fb; }
