#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include <gdt.hpp>
#include <printf.hpp>
#include <idt.hpp>

#define restrict __restrict

extern "C" {
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

    void kmain(void);
}

extern "C" void printf(const char* str);
extern "C" void init_printf(struct limine_framebuffer* fb, uint32_t text_color, uint32_t background_color);
extern "C" void remapPic();

void *memcpy(void *restrict dest, const void *restrict src, size_t n)
{
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    for (size_t i = 0; i < n; i++)
    {
        pdest[i] = psrc[i];
    }

    return dest;
}

void *memset(void *s, int c, size_t n)
{
    uint8_t *p = (uint8_t *)s;

    for (size_t i = 0; i < n; i++)
    {
        p[i] = (uint8_t)c;
    }

    return s;
}

void *memmove(void *dest, const void *src, size_t n)
{
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

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
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

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

extern "C" void DrawBox(struct limine_framebuffer *fb, uint64_t startX, uint64_t startY, uint64_t width, uint64_t height, uint8_t r, uint8_t g, uint8_t b)
{
    volatile uint32_t *fbPtr = (volatile uint32_t *)fb->address;
    
    uint32_t color = FramebufferPixel(fb, r, g, b);
    
    uint64_t pitchPixels = fb->pitch / 4;

    for (uint64_t y = startY; y < startY + height; y++)
    {
        if (y >= fb->height) break;

        for (uint64_t x = startX; x < startX + width; x++)
        {
            if (x >= fb->width) break;
            fbPtr[y * pitchPixels + x] = color;
        }
    }
}

static uint8_t emergencyKernelStack[16384];
static GlobalDescriptorTable kernelGDT;
static InterruptDescriptorTable kernelIDT;

void kmain(void)
{
    asm volatile("cli"); 

    uint64_t stackTop = reinterpret_cast<uint64_t>(emergencyKernelStack) + sizeof(emergencyKernelStack);
    kernelGDT.init(stackTop);
    kernelGDT.load();

    kernelIDT.init();
    kernelIDT.load();

    remapPic();

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
    }

    struct limine_framebuffer *fb = FramebufferRequest.response->framebuffers[0];
    DrawBox(fb, 0, 0, fb->width, fb->height, 0, 0, 255);

    init_printf(fb, 0xFFFFFFFF, 0x000000FF);
    printf("Welcome to LunarOS Version 1.2!\n");
    
    asm volatile ("sti");
    
    asm volatile ("int $0x20");

    hcf();
}
