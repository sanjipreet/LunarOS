#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

__attribute__((used, section(".limine_requests")))
static volatile uint64_t LimineBaseRevision[] = LIMINE_BASE_REVISION(6);

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request FramebufferRequest = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t LimineRequestStartMaker[] = LIMINE_REQUESTS_START_MARKER;


__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t LimineRequestEndMaker[] = LIMINE_REQUESTS_END_MARKER;

void *memcpy(void *restrict dest, const void *restrict src, size_t n)
{
    uint8_t *pdest = dest;
    const uint8_t *psrc = src;

    for (size_t i = 0; i < n; i++)
    {
        pdest[i] = psrc[i];
    }

    return dest;
}

void *memset(void *s, int c, size_t n)
{
    uint8_t *p = s;

    for (size_t i = 0; i < n; i++)
    {
        p[i] = (uint8_t)c;
    }

    return s;
}

void *memmove(void *dest, const void *src, size_t n)
{
    uint8_t *pdest = dest;
    const uint8_t *psrc = src;

    if ((uintptr_t)src > (uintptr_t)dest)
    {
        for (size_t i = 0; i < n; i++)
        {
            pdest[i] = psrc[i];
        }
    } else if ((uintptr_t)src < (uintptr_t)dest)
    {
        for (size_t i = n; i > 0; i--)
        {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
    const uint8_t *p1 = s1;
    const uint8_t *p2 = s2;

    for (size_t i = 0; i < n; i++)
    {
        if (p1[i] != p2[i])
        {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

static void hcf(void)
{
    for (;;)
    {
        asm ("hlt");
    }
}

static uint32_t FramebufferChannel(uint8_t value, uint8_t maskSize, uint8_t maskSift)
{
    uint64_t max = ((uint64_t)1 << maskSize) - 1;
    return (uint32_t)((value * max / 255) << maskSift);
}

static uint32_t FramebufferPixel(struct limine_framebuffer *fb, uint8_t red, uint8_t green, uint8_t blue) {
    return FramebufferChannel(red, fb->red_mask_size, fb->red_mask_shift)
         | FramebufferChannel(green, fb->green_mask_size, fb->green_mask_shift)
         | FramebufferChannel(blue, fb->blue_mask_size, fb->blue_mask_shift);
}

static uint32_t FramebufferPattern(struct limine_framebuffer *fb)
{
    volatile uint32_t *fbPtr = fb->address;
    
    for(signed y = 0; y < fb->height; y++)
    {
        for (size_t x = 0; x < fb->width; x++)
        {
            uint8_t nX = x * 255 / fb->width;
            uint8_t nY = y * 255 / fb->height;
            fbPtr[y * (fb->pitch / 4) + x] = FramebufferPixel(fb, 0, nY, nX);
        }
    }
}

void kmain(void)
{
    if (LIMINE_BASE_REVISION_SUPPORTED(LimineBaseRevision) == false)
    {
        hcf();
    }

    if (FramebufferRequest.response == NULL || FramebufferRequest.response->framebuffer_count < 1)
    {
        hcf();
    }

    for (uint64_t i = 0; i < FramebufferRequest.response->framebuffer_count; i++)
    {
        struct limine_framebuffer *Framebuffer = FramebufferRequest.response->framebuffers[i];

        if (Framebuffer->memory_model != LIMINE_FRAMEBUFFER_RGB || Framebuffer->bpp != 32)
        {
            hcf();
        }

        FramebufferPattern(Framebuffer);
    }

    hcf();
}