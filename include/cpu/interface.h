#ifndef CPU_INTERFACE_H
#define CPU_INTERFACE_H

#include "core/memory.h"
#include <string>

constexpr uint32_t PI_MMIO_BASE = 0x0C003000;
constexpr uint32_t PI_MMIO_END = 0x0C0030FF;

enum class PIInterrupt : uint32_t {
    GPError = 0,
    Reset = 1,
    DVD = 2,
    SI = 3,
    EXI = 4,
    AI = 5,
    DSP = 6,
    MEM = 7,
    VI = 8,
    PEToken = 9,
    PEFinish = 10,
    CP = 11,
    Debug = 12,
    HSP = 13,
    Hollywood = 14,
};

class ProcessorInterface : public MMIODevice {
  public:
    uint32_t read(uint32_t offset, AccessSize size) override;
    void write(uint32_t offset, uint32_t value, AccessSize size) override;
    inline std::string getName() const { return "ProcessorInterface"; }

    inline bool interruptPending() const {
        return (interruptCause & interruptMask & 0x0000FFFF) != 0;
    }

    inline void raiseIntererupt(PIInterrupt interrupt) {
        interruptCause |= (1u << static_cast<uint32_t>(interrupt));
    }

    inline void clearInterrupt(PIInterrupt interrupt) {
        interruptCause &= ~(1u << static_cast<uint32_t>(interrupt));
    }

  private:
    uint32_t interruptMask;
    uint32_t interruptCause;
};

#endif