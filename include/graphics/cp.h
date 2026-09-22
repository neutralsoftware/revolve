#ifndef COMMAND_PROCESSOR_H
#define COMMAND_PROCESSOR_H

#include "core/memory.h"
#include <cstdint>
#include <string>

constexpr uint32_t CP_BASE = 0x0C000000;
constexpr uint32_t CP_SIZE = 0x80;

enum class CPRegister : uint32_t {
    Status = 0x00,
    Control = 0x02,
    Clear = 0x04,

    Token = 0x0E,

    FifoStartHi = 0x20,
    FifoStartLo = 0x22,

    FifoEndHi = 0x24,
    FifoEndLo = 0x26,

    FifoHighWatermarkHi = 0x28,
    FifoHighWatermarkLo = 0x2A,

    FifoLowWatermarkHi = 0x2C,
    FifoLowWatermarkLo = 0x2E,

    FifoReadWriteDistanceHi = 0x30,
    FifoReadWriteDistanceLo = 0x32,

    FifoWritePointerHi = 0x34,
    FifoWritePointerLo = 0x36,

    FifoReadPointerHi = 0x38,
    FifoReadPointerLo = 0x3A,

    FifoBreakpointHi = 0x3C,
    FifoBreakpointLo = 0x3E,
};

struct CPFifo {
    uint32_t base = 0;
    uint32_t end = 0;

    uint32_t highWatermark = 0;
    uint32_t lowWatermark = 0;

    uint32_t readWriteDistance = 0;

    uint32_t writePointer = 0;
    uint32_t readPointer = 0;

    uint32_t breakpoint = 0;
};

struct CPState {
    bool bpInterrupt = false;
    bool commandIdle = true;
    bool readIdle = true;
    bool underflow = false;
    bool overflow = false;

    bool breakpointEnable = false;
    bool fifoLinkEnable = true;
    bool underflowInterruptEnable = false;
    bool overflowInterruptEnable = true;
    bool cpInterruptEnable = false;
    bool fifoReadEnable = true;

    uint16_t token = 0;

    CPFifo fifo{};
};

constexpr uint16_t FIFO_HI_MASK = 0x1FFF;
constexpr uint16_t FIFO_LO_MASK = 0xFFE0;

class CommandProcessor : public MMIODevice {
  public:
    uint32_t read(uint32_t offset, AccessSize size) override;
    void write(uint32_t offset, uint32_t value, AccessSize size) override;

    std::string getName() override { return "Command Processor"; }

    inline static void writeHigh(uint32_t &target, uint16_t value) {
        target = (target & 0x0000FFFF) |
                 (static_cast<uint32_t>(value & FIFO_HI_MASK) << 16);
    }

    static void writeLow(uint32_t &target, uint16_t value) {
        target =
            (target & 0xFFFF0000) | static_cast<uint32_t>(value & FIFO_LO_MASK);
    }

    CPFifo &getFifo() { return state.fifo; }
    bool isFifoReadEnabled() const { return state.fifoReadEnable; }

    void onGatherPipeBurst();

    void onFifoBlockConsumed();

  private:
    uint16_t read16(uint32_t offset) const;
    void write16(uint32_t offset, uint16_t value);

    uint32_t calculateFifoDistance() const;

    void updateStatus();
    void updateInterrupt();

    CPState state{};
};

#endif
