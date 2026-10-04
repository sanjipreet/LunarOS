#include <idt.hpp>
#include <port.hpp>
#include <printf.hpp>

#define PIC1_COMMAND 0x20

__attribute__((aligned(0x10)))
IDTEntry InterruptDescriptorTable::idtTable[256];
IDTPointer InterruptDescriptorTable::idtPointer;

InterruptHandler* InterruptDescriptorTable::handlers[256];

extern "C" volatile uint8_t active_interrupt_vector;

InterruptHandler::InterruptHandler(InterruptDescriptorTable* manager, uint8_t interruptNum) {
    this->interruptNumber = interruptNum;
    this->tableManager = manager;
    InterruptDescriptorTable::handlers[interruptNum] = this;
}

InterruptHandler::~InterruptHandler() {
    if (InterruptDescriptorTable::handlers[interruptNumber] == this) {
        InterruptDescriptorTable::handlers[interruptNumber] = nullptr;
    }
}

void InterruptDescriptorTable::setGate(uint8_t vector, void (*handler)(), uint8_t attributes) {
    uint64_t handlerAddress = reinterpret_cast<uint64_t>(handler);
    idtTable[vector].offsetLow = handlerAddress & 0xFFFF;
    idtTable[vector].selector = 0x08; // Point directly to our Kernel Code Segment (Entry 1)
    idtTable[vector].ist = 0;
    idtTable[vector].typeAttributes = attributes; // 0x8E for standard active Ring 0 Gate
    idtTable[vector].offsetMiddle = (handlerAddress >> 16) & 0xFFFF;
    idtTable[vector].offsetHigh = (handlerAddress >> 32) & 0xFFFFFFFF;
    idtTable[vector].reserved = 0;
}

void InterruptDescriptorTable::init() {
    for (int i = 0; i < 256; i++) {
        idtTable[i] = {0, 0, 0, 0, 0, 0, 0};
        handlers[i] = nullptr; 
    }

    setGate(0x20, HandleInterruptIgnore0x20, 0x8E);
    setGate(0x21, HandleInterruptIgnore0x21, 0x8E);

    idtPointer.size = sizeof(idtTable) - 1;
    idtPointer.offset = reinterpret_cast<uint64_t>(&idtTable);
}

void InterruptDescriptorTable::load() {
    asm volatile("lidt %0" : : "m"(idtPointer));
}

extern "C" void coreInterruptDispatcher() {
    uint8_t vector = active_interrupt_vector;

    if (InterruptDescriptorTable::handlers[vector] != nullptr) {
        InterruptDescriptorTable::handlers[vector]->handleInterrupt();
    }

    if (vector == 0x20 || vector == 0x21) {
        printf("[LunarOS] Successfully got a clock interupt!\n");
        Port::outb(PIC1_COMMAND, 0x20);
    }
}
