#include "ios/ipc.h"
#include "device.h"
#include <cstdint>

uint32_t IPC::read(uint32_t offset, AccessSize size) {
    switch (offset) {
    case IPC_PPCMSG:
        return ppcMessage;
    case IPC_PPCCTRL: {
        uint32_t value = 0;

        if (x1)
            value |= 1u << 0;
        if (y2)
            value |= 1u << 1;
        if (y1)
            value |= 1u << 2;
        if (x2)
            value |= 1u << 3;

        if (interruptY1)
            value |= 1u << 4;
        if (interruptY2)
            value |= 1u << 5;

        return value;
    }
    case IPC_ARMMSG:
        return armMessage;
    case IPC_ARMCTRL:
        return 0; // For HLE, we don't need to emulate the ARM control register.
    default:
        Logger::log("IPC", LogLevel::Warning,
                    "Unimplemented IPC read: 0x" + utils::toHexString(offset));
        return 0;
    }
}

void IPC::write(uint32_t offset, uint32_t value, AccessSize size) {
    switch (offset) {
    case IPC_PPCMSG:
        ppcMessage = value;
        break;

    case IPC_PPCCTRL:
        x1 = (value & (1u << 0)) != 0;
        x2 = (value & (1u << 3)) != 0;

        if (value & (1u << 1))
            y2 = false;

        if (value & (1u << 2))
            y1 = false;

        interruptY1 = (value & (1u << 4)) != 0;
        interruptY2 = (value & (1u << 5)) != 0;

        updateInterrupts();
        break;

    case IPC_ARMMSG:
        armMessage = value;
        break;
    case IPC_ARMCTRL:
        // For HLE, we don't need to emulate the ARM control register.
        break;
    default:
        Logger::log("IPC", LogLevel::Warning,
                    "Unimplemented IPC write: 0x" + utils::toHexString(offset));
        return;
    }
}

void IPC::replyFromStarlet(uint32_t requestAddress) {
    armMessage = requestAddress;
    y1 = true;
    updateInterrupts();
}

void IPC::updateInterrupts() {
    bool shouldInterrupt = (y1 && interruptY1) || (y2 && interruptY2);

    auto &hollywood = Device::globalDevice->controller;

    if (shouldInterrupt)
        hollywood.raise(HollywoodIRQ::IPC);
    else
        hollywood.clear(HollywoodIRQ::IPC);
}

bool IPC::ppcRequestPending() const { return x1; }

uint32_t IPC::getPPCMessage() const { return ppcMessage; }
