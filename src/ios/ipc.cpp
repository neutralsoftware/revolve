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
            value |= 1 << 0;
        if (x2)
            value |= 1 << 1;
        if (y1)
            value |= 1 << 2;
        if (y2)
            value |= 1 << 3;

        if (interruptY1)
            value |= 1 << 4;
        if (interruptY2)
            value |= 1 << 5;

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
        if (value & (1 << 0))
            x1 = true;

        if (value & (1 << 3))
            x2 = true;

        if (value & (1 << 1))
            y2 = false;

        if (value & (1 << 2))
            y1 = false;

        interruptY1 = value & (1 << 4);
        interruptY2 = value & (1 << 5);

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
    bool shouldInterrupt = y1 && interruptY1;

    auto hollywood = Device::globalDevice->controller;

    if (shouldInterrupt)
        hollywood.raise(HollywoodIRQ::IPC);
    else
        hollywood.clear(HollywoodIRQ::IPC);
}