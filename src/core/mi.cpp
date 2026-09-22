#include "cpu/memory_interface.h"
#include <stdexcept>

uint32_t MemoryInterface::read(uint32_t offset, AccessSize size) {
    if (size == AccessSize::U16)
        return registers[offset >> 1];
    if (size == AccessSize::U32)
        return (static_cast<uint32_t>(registers[offset >> 1]) << 16) |
               registers[(offset >> 1) + 1];
    throw std::runtime_error(
        "Memory Interface only supports 16-bit and 32-bit accesses");
}

void MemoryInterface::write(uint32_t offset, uint32_t value, AccessSize size) {
    if (size == AccessSize::U16) {
        registers[offset >> 1] = static_cast<uint16_t>(value);
        return;
    }
    if (size == AccessSize::U32) {
        registers[offset >> 1] = static_cast<uint16_t>(value >> 16);
        registers[(offset >> 1) + 1] = static_cast<uint16_t>(value);
        return;
    }
    throw std::runtime_error(
        "Memory Interface only supports 16-bit and 32-bit accesses");
}
