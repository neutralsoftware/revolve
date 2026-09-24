
#include "input/serial.h"
#include "cpu/interface.h"
#include "device.h"

uint32_t SerialInterface::read(uint32_t offset, AccessSize size) {
    if (offset >= 0x80 && offset < 0x100) {
        const uint32_t index = offset - 0x80;

        if (size == AccessSize::U32) {
            if (index + 3 >= communicationBuffer.size())
                return 0;

            return (uint32_t(communicationBuffer[index + 0]) << 24) |
                   (uint32_t(communicationBuffer[index + 1]) << 16) |
                   (uint32_t(communicationBuffer[index + 2]) << 8) |
                   uint32_t(communicationBuffer[index + 3]);
        }

        return 0;
    }

    if (size != AccessSize::U32) {
        Logger::log("SI", LogLevel::Warning, "Unsupported SI access size");

        return 0;
    }

    if (offset < 0x30) {
        const uint32_t channel = offset / 0x0C;
        const uint32_t reg = offset % 0x0C;

        if (channel < 4) {
            switch (reg) {
            case 0x00:
                return channels[channel].outBuffer;
            case 0x04:
                return channels[channel].inBufferHi;
            case 0x08:
                return channels[channel].inBufferLo;
            }
        }
    }

    switch (offset) {
    case 0x30:
        return poll;
    case 0x34:
        return comcsr;
    case 0x38:
        return status;
    case 0x3C:
        return exiClockLock;

    default:
        return 0;
    }
}

void SerialInterface::write(uint32_t offset, uint32_t value, AccessSize size) {
    if (size != AccessSize::U32)
        return;

    if (offset < 0x30) {
        const uint32_t channel = offset / 0x0C;
        const uint32_t reg = offset % 0x0C;

        if (channel < 4 && reg == 0x00) {
            channels[channel].outBuffer = value;
            return;
        }
    }

    switch (offset) {
    case 0x30:
        poll = value;
        return;
    case 0x34:
        handleComCSRWrite(value);
        return;
    case 0x38:
        handleStatusWrite(value);
        return;
    case 0x3C:
        exiClockLock = value;
        return;

    default:
        return;
    }
}

void SerialInterface::updatePoll() {
    for (unsigned channel = 0; channel < channels.size(); ++channel) {
        if (!channels[channel].device)
            continue;
        pollChannel(channel);
    }
}

void SerialInterface::step(uint64_t ticks) {
    accumulatedTicks += ticks;

    while (accumulatedTicks >= pollInterval) {
        accumulatedTicks -= pollInterval;

        updatePoll();
    }
}

void SerialInterface::pollChannel(unsigned channel) {
    if (channel >= channels.size())
        return;

    auto &ch = channels[channel];

    if (!ch.device)
        return;

    const uint32_t command = ch.outBuffer;

    uint8_t request[3] = {
        static_cast<uint8_t>((command >> 16) & 0xFF),
        static_cast<uint8_t>((command >> 8) & 0xFF),
        static_cast<uint8_t>(command & 0xFF),
    };

    uint8_t response[8]{};

    const int responseSize = ch.device->runCommand(request, sizeof(request),
                                                   response, sizeof(response));

    if (responseSize < 8)
        return;

    ch.inBufferHi = (uint32_t(response[0]) << 24) |
                    (uint32_t(response[1]) << 16) |
                    (uint32_t(response[2]) << 8) | uint32_t(response[3]);
    ch.inBufferLo = (uint32_t(response[4]) << 24) |
                    (uint32_t(response[5]) << 16) |
                    (uint32_t(response[6]) << 8) | uint32_t(response[7]);

    setChannelReadStatus(channel);

    updateInterrupt();
}

void SerialInterface::updateInterrupt() {
    using namespace SIComCSR;

    const bool communicationInterrupt =
        (comcsr & TransferComplete) && (comcsr & InterruptMask);

    const bool pollingInterrupt = status != 0;

    const bool interrupt = communicationInterrupt || pollingInterrupt;

    if (interrupt) {
        Device::globalDevice->pi->raiseInterrupt(PIInterrupt::SI);
    } else {
        Device::globalDevice->pi->clearInterrupt(PIInterrupt::SI);
    }
}

void SerialInterface::attachDevice(
    unsigned channel, std::shared_ptr<SerialInterfaceDevice> device) {
    if (channel >= channels.size()) {
        Logger::log("SI", LogLevel::Warning,
                    "Tried to attach SI device to invalid channel");

        return;
    }

    channels[channel].device = std::move(device);
}

void SerialInterface::setChannelReadStatus(unsigned channel) {
    if (channel >= 4)
        return;

    status |= CHANNEL_READ_STATUS_BITS[channel];
}

void SerialInterface::handleStatusWrite(uint32_t value) {
    status &= ~value;

    updateInterrupt();
}

void SerialInterface::handleComCSRWrite(uint32_t value) {
    using namespace SIComCSR;

    if (value & TransferComplete) {
        comcsr &= ~TransferComplete;
    }

    const uint32_t writable =
        InterruptMask | ChannelMask | OutLengthMask | InLengthMask;

    comcsr = (comcsr & ~writable) | (value & writable);

    if (value & TransferStart) {
        comcsr |= TransferStart;

        executeTransfer();
    }

    updateInterrupt();
}

void SerialInterface::executeTransfer() {
    using namespace SIComCSR;

    const unsigned channel = (comcsr & ChannelMask) >> ChannelShift;

    unsigned outLength = (comcsr & OutLengthMask) >> OutLengthShift;
    unsigned inLength = (comcsr & InLengthMask) >> InLengthShift;

    if (channel >= channels.size()) {
        comcsr &= ~TransferStart;
        comcsr |= TransferComplete;

        updateInterrupt();
        return;
    }

    auto &ch = channels[channel];

    if (!ch.device) {
        comcsr &= ~TransferStart;
        comcsr |= TransferComplete;

        updateInterrupt();
        return;
    }

    if (outLength == 0)
        outLength = 128;

    if (inLength == 0)
        inLength = 128;

    outLength = std::min<unsigned>(outLength, communicationBuffer.size());

    inLength = std::min<unsigned>(inLength, communicationBuffer.size());

    std::array<uint8_t, 128> response{};

    const int responseSize = ch.device->runCommand(
        communicationBuffer.data(), outLength, response.data(), inLength);

    if (responseSize > 0) {
        const size_t bytes =
            std::min<size_t>(static_cast<size_t>(responseSize), inLength);

        std::copy_n(response.begin(), bytes, communicationBuffer.begin());
    }

    comcsr &= ~TransferStart;
    comcsr |= TransferComplete;

    updateInterrupt();
}