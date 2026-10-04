#pragma once
#include <stdint.h>

struct IDTEntry {
    uint16_t offsetLow;
    uint16_t selector;
    uint8_t  ist;
    uint8_t  typeAttributes;
    uint16_t offsetMiddle;
    uint32_t offsetHigh;
    uint32_t reserved;
} __attribute__((packed));

struct IDTPointer {
    uint16_t size;
    uint64_t offset; 
} __attribute__((packed));

class InterruptDescriptorTable;

class InterruptHandler {
protected:
    uint8_t interruptNumber;
    InterruptDescriptorTable* tableManager;

    InterruptHandler(InterruptDescriptorTable* manager, uint8_t interruptNum);
    virtual ~InterruptHandler();

public:
    virtual void handleInterrupt() = 0;
};

class InterruptDescriptorTable {
friend class InterruptHandler;
private:
    static IDTEntry idtTable[256];
    static IDTPointer idtPointer;
    
    void setGate(uint8_t vector, void (*handler)(), uint8_t attributes);

public:
    static InterruptHandler* handlers[256];

    InterruptDescriptorTable() = default;
    void init();
    void load();
    
    static void executeHandler(uint8_t vector);
};

extern "C" {
    void HandleInterruptIgnore0x20();
    void HandleInterruptIgnore0x21();
    void coreInterruptDispatcher();
}
