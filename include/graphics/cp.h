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

    FifoStartHi = 0x22,
    FifoStartLo = 0x20,

    FifoEndHi = 0x26,
    FifoEndLo = 0x24,

    FifoHighWatermarkHi = 0x2A,
    FifoHighWatermarkLo = 0x28,

    FifoLowWatermarkHi = 0x2E,
    FifoLowWatermarkLo = 0x2C,

    FifoReadWriteDistanceHi = 0x32,
    FifoReadWriteDistanceLo = 0x30,

    FifoWritePointerHi = 0x36,
    FifoWritePointerLo = 0x34,

    FifoReadPointerHi = 0x3A,
    FifoReadPointerLo = 0x38,

    FifoBreakpointHi = 0x3E,
    FifoBreakpointLo = 0x3C,
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
    bool fifoLinkEnable = false;
    bool underflowInterruptEnable = false;
    bool overflowInterruptEnable = false;
    bool cpInterruptEnable = false;
    bool fifoReadEnable = false;

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

    bool canReadFifo();
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

class PixelEngine : public MMIODevice {
  public:
    uint32_t read(uint32_t offset, AccessSize size) override;
    void write(uint32_t offset, uint32_t value, AccessSize size) override;
    std::string getName() override { return "Pixel Engine"; }
    void setToken(uint16_t value, bool interrupt);
    void finish();

  private:
    void updateInterrupts();
    uint16_t registers[5]{};
    uint16_t control = 0;
    uint16_t token = 0;
    bool tokenPending = false;
    bool finishPending = false;
};

#endif
