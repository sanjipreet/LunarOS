#include <gdt.hpp>

void GlobalDescriptorTable::setEntry(GDTEntry& target, uint8_t access, uint8_t flags)
{
    target.limitLow = 0;
    target.baseLow = 0;
    target.baseMiddle = 0;
    target.flagsLimitHigh = flags & 0xF0;
    target.accessByte = access;
    target.baseHigh = 0;
}

void GlobalDescriptorTable::setTssEntry(uint64_t tssAddress)
{
    uint32_t tssSize = sizeof(TaskStateSegment) - 1;

    entries.tssDesc.limitLow = tssSize & 0xFFFF;
    entries.tssDesc.baseLow = tssAddress & 0xFFFF;
    entries.tssDesc.baseMiddle = (tssAddress >> 16) & 0xFF;
    entries.tssDesc.accessBytes = 0x89;
    entries.tssDesc.flagsLimitHigh = ((tssSize >> 16) & 0x0F);
    entries.tssDesc.baseHigh = (tssAddress >> 24) & 0xFF;
    entries.tssDesc.baseHighest = (tssAddress >> 32) & 0xFFFFFFFF;
    entries.tssDesc.reserved = 0;
}

void GlobalDescriptorTable::init(uint64_t kernelStackAddress)
{
    for (size_t i = 0; i < sizeof(TaskStateSegment); i++) {
        reinterpret_cast<uint8_t*>(&tss)[i] = 0;
    }

    tss.rsp0 = kernelStackAddress;
    tss.iomapBase = sizeof(TaskStateSegment);

    setEntry(entries.nullDesc, 0, 0);
    setEntry(entries.kernelCode, 0x9A, 0x20);
    setEntry(entries.kernelData, 0x92, 0x00);
    setEntry(entries.userData, 0xF2, 0x00);
    setEntry(entries.userCode, 0xFA, 0x20);

    setTssEntry(reinterpret_cast<uint64_t>(&tss));

    pointer.size = sizeof(entries) - 1;
    pointer.offset = reinterpret_cast<uint64_t>(&entries);
}

void GlobalDescriptorTable::load()
{
    gdtFlushSegments(&pointer);
    tssFlushSelector(0x28);
}