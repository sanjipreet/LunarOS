#ifndef PORT_HPP
#define PORT_HPP

    #include <stdint.h>

    class Port {
        
        public:

                static void outb(uint16_t port, uint8_t data);
                static uint8_t inb(uint16_t port);

                static void outw(uint16_t port, uint16_t data);
                static uint16_t inw(uint16_t port);

                static void outl(uint16_t port, uint32_t data);
                static uint32_t inl(uint16_t port);

                static inline void ioWait() 
                {
                    outb(0x80, 0);
                }
    };

#endif