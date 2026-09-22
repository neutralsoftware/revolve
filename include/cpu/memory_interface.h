#ifndef REVOLVE_MEMORY_INTERFACE
#define REVOLVE_MEMORY_INTERFACE

#include "core/memory.h"
#include <array>
#include <cstdint>
#include <string>

constexpr uint32_t MI_MMIO_BASE = 0x0C004000;
constexpr uint32_t MI_MMIO_END = 0x0C004FFF;

class MemoryInterface : public MMIODevice {
  public:
    uint32_t read(uint32_t offset, AccessSize size) override;
    void write(uint32_t offset, uint32_t value, AccessSize size) override;
    inline std::string getName() override { return "MemoryInterface"; }

  private:
    std::array<uint16_t, 0x800> registers = {};
};

#endif
