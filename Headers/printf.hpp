#ifndef PRINTF_HPP
#define PRINTF_HPP

    #include <stddef.h>
    #include <stdint.h>
    #include <limine.h>

    class FramebufferConsole {
        
        private:

            struct limine_framebuffer *fb;
            uint32_t cursorX;
            uint32_t cursorY;
            uint32_t fontColor;
            uint32_t bgColor;

            inline void drawPixel(uint32_t x, uint32_t y, uint32_t color);

        public:

                FramebufferConsole() = default;

                void init(struct limine_framebuffer* framebuffer, uint32_t text_color, uint32_t background_color);
                void putChar(char c);
                void print(const char* str);
    };

extern "C" void printf(const char* str);

#endif