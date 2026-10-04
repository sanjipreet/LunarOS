// C++/pic.cpp
#include <port.hpp>

#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

#define ICW1_INIT    0x11
#define ICW4_8086    0x01

extern "C" void remapPic() {
    uint8_t a1 = Port::inb(PIC1_DATA);
    uint8_t a2 = Port::inb(PIC2_DATA);

    Port::outb(PIC1_COMMAND, ICW1_INIT);
    Port::ioWait();
    Port::outb(PIC2_COMMAND, ICW1_INIT);
    Port::ioWait();

    Port::outb(PIC1_DATA, 0x20);
    Port::ioWait();
    Port::outb(PIC2_DATA, 0x28);
    Port::ioWait();

    Port::outb(PIC1_DATA, 4);
    Port::ioWait();
    Port::outb(PIC2_DATA, 2);
    Port::ioWait();

    Port::outb(PIC1_DATA, ICW4_8086);
    Port::ioWait();
    Port::outb(PIC2_DATA, ICW4_8086);
    Port::ioWait();

    Port::outb(PIC1_DATA, 0xFE); 
    Port::outb(PIC2_DATA, 0xFF); // Keep all slave interrupts fully masked
}
