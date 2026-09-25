
#pragma once

#include "core/memory.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

class SerialInterfaceDevice {
  public:
    virtual ~SerialInterfaceDevice() = default;

    virtual int runCommand(const uint8_t *request, size_t requestSize,
                           uint8_t *response, size_t responseCapacity) = 0;

    virtual void sendDirectCommand(uint32_t command, uint8_t poll) {};
};

namespace SIComCSR {
constexpr uint32_t TransferStart = 1u << 0;
constexpr uint32_t TransferComplete = 1u << 1;
constexpr uint32_t InterruptMask = 1u << 2;

constexpr uint32_t ChannelShift = 8;
constexpr uint32_t ChannelMask = 0x3u << ChannelShift;

constexpr uint32_t OutLengthShift = 16;
constexpr uint32_t OutLengthMask = 0x7Fu << OutLengthShift;

constexpr uint32_t InLengthShift = 24;
constexpr uint32_t InLengthMask = 0x7Fu << InLengthShift;
} // namespace SIComCSR

class SerialInterface : public MMIODevice {
  public:
    uint32_t read(uint32_t offset, AccessSize size) override;
    void write(uint32_t offset, uint32_t value, AccessSize size) override;
    std::string getName() override { return "SerialInterface"; }

    void step(uint64_t ticks);

    void attachDevice(unsigned channel,
                      std::shared_ptr<SerialInterfaceDevice> device);

    void pollChannel(unsigned channel);

  private:
    struct Channel {
        uint32_t outBuffer = 0;
        uint32_t inBufferHi = 0;
        uint32_t inBufferLo = 0;

        std::shared_ptr<SerialInterfaceDevice> device;
    };

    std::array<Channel, 4> channels{};

    uint32_t poll = 0;
    uint32_t comcsr = 0;
    uint32_t status = 0;
    uint32_t exiClockLock = 0;

    std::array<uint8_t, 128> communicationBuffer{};

    void updatePoll();
    void executeTransfer();
    void updateInterrupt();

    void handleComCSRWrite(uint32_t value);
    void handleStatusWrite(uint32_t value);

    void setChannelReadStatus(unsigned channel);

    uint64_t accumulatedTicks;
    static constexpr uint32_t pollInterval = 1000;

    static constexpr uint32_t CHANNEL_READ_STATUS_BITS[4] = {
        1u << 29,
        1u << 21,
        1u << 13,
        1u << 5,
    };
};