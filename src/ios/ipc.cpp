#include "ios/ipc.h"
#include "device.h"
#include <cstdint>

uint32_t IPC::read(uint32_t offset, AccessSize size) {
    switch (offset) {
    case 0x30:
        return Device::globalDevice->controller.getFlags();
    case 0x34:
        return Device::globalDevice->controller.getMask();
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
    case 0x64:
        return 0xFFFFFFFF;
    case 0xC0:
    case 0xE0:
        return gpioOutput;
    case 0xC4:
    case 0xE4:
        return gpioDirection;
    case 0xC8:
    case 0xE8:
        return Device::globalDevice->disc->isOpen() ? 0x80 : 0;
    case 0x180:
    case 0x1CC:
    case 0x1D0:
        return 0;
    case 0x194:
        return hardwareResets;
    default:
        Logger::log("IPC", LogLevel::Warning,
                    "Unimplemented IPC read: 0x" + utils::toHexString(offset));
        return 0;
    }
}

void IPC::write(uint32_t offset, uint32_t value, AccessSize size) {
    switch (offset) {
    case 0x30:
        Device::globalDevice->controller.acknowledge(value);
        updateInterrupts();
        break;
    case 0x34:
        Device::globalDevice->controller.setMask(value);
        updateInterrupts();
        break;
    case IPC_PPCMSG:
        ppcMessage = value;
        break;

    case IPC_PPCCTRL:
        x1 = (value & (1u << 0)) != 0;
        x2 = (value & (1u << 3)) != 0;

        if (value & (1u << 1))
            y2 = false;

        if (value & (1u << 2)) {
            y1 = false;
            presentReply();
        }

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
    case 0x18:
    case 0x24:
    case 0x64:
    case 0xC8:
    case 0xE8:
        break;
    case 0xC0:
        gpioOutput = (gpioOutput & ~0xC3A0) | (value & 0xC3A0);
        break;
    case 0xC4:
        gpioDirection = (gpioDirection & ~0xC3A0) | (value & 0xC3A0);
        break;
    case 0xE0:
        gpioOutput = (gpioOutput & 0xC3A0) | (value & ~0xC3A0);
        break;
    case 0xE4:
        gpioDirection = (gpioDirection & 0xC3A0) | (value & ~0xC3A0);
        break;
    case 0x180:
    case 0x1CC:
    case 0x1D0:
        break;
    case 0x194:
        hardwareResets = value;
        break;
    default:
        Logger::log("IPC", LogLevel::Warning,
                    "Unimplemented IPC write: 0x" + utils::toHexString(offset));
        return;
    }
}

void IPC::replyFromStarlet(uint32_t requestAddress) {
    pendingReplies.push_back(requestAddress);
    presentReply();
    updateInterrupts();
}

void IPC::presentReply() {
    if (y1 || pendingReplies.empty())
        return;
    armMessage = pendingReplies.front();
    pendingReplies.pop_front();
    y1 = true;
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
