#ifndef GDT_HPP
#define GDT_HPP

    #include <stddef.h>
    #include <stdint.h>

     struct GDTEntry {
        uint16_t limitLow;
        uint16_t baseLow;
        uint8_t baseMiddle;
        uint8_t accessByte;
        uint8_t flagsLimitHigh;
        uint8_t baseHigh;
    } __attribute__((packed));

     struct GDTSystemEntry {
        uint16_t limitLow;
        uint16_t baseLow;
        uint8_t baseMiddle;
        uint8_t accessBytes;
        uint8_t flagsLimitHigh;
        uint8_t baseHigh;
        uint32_t baseHighest;
        uint32_t reserved;
    } __attribute__((packed));

     struct TaskStateSegment {
        uint32_t reserved0; 
        uint64_t rsp0;
        uint64_t rsp1;
        uint64_t rsp2;
        uint64_t reserved1;
        uint64_t ist[7];
        uint64_t reserved2;
        uint16_t reserved3;
        uint16_t iomapBase;
    } __attribute__((packed));

     struct GDTPointer {
        uint16_t size;
        uint64_t offset;
    } __attribute__((packed));

    extern "C" {
        void gdtFlushSegments(GDTPointer* ptr);
        void tssFlushSelector(uint16_t selector);
    }

    class GlobalDescriptorTable {
        private:

            struct {
                GDTEntry nullDesc;
                GDTEntry kernelCode;
                GDTEntry kernelData;
                GDTEntry userData;
                GDTEntry userCode;
                GDTSystemEntry tssDesc;
            } __attribute__((packed)) entries;

            GDTPointer pointer;
            TaskStateSegment tss;

            void setEntry(GDTEntry& target, uint8_t access, uint8_t flags);
            void setTssEntry(uint64_t tssAddress);

        public:
            GlobalDescriptorTable() = default;

            void init(uint64_t kernelStackAddress);
            void load();
    };

#endif