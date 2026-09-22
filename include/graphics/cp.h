#ifndef COMMAND_PROCESSOR_H
#define COMMAND_PROCESSOR_H

#include "core/memory.h"
#include <cstdint>
#include <string>
#include <sys/qos.h>

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

    FifoWritePointerHi = 0x34,
    FifoWritePointerLo = 0x36,
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

    uint32_t fifoStart = 0;
    uint32_t fifoEnd = 0;
    uint32_t fifoWritePointer = 0;
};

struct CPFifo {
    uint32_t start = 0;
    uint32_t end = 0;

    uint32_t readPointer = 0;
    uint32_t writePointer = 0;

    uint32_t distance = 0;

    uint32_t highWatermark = 0;
    uint32_t lowWatermark = 0;

    uint32_t breakpoint = 0;
};

class CommandProcessor : public MMIODevice {
  public:
    uint32_t read(uint32_t offset, AccessSize size) override;
    void write(uint32_t offset, uint32_t value, AccessSize size) override;

    std::string getName() override { return "Command Processor"; }

  private:
    uint16_t read16(uint32_t offset) const;
    void write16(uint32_t offset, uint16_t value);

    CPState state{};
};

#endif // COMMAND_PROCESSOR_H