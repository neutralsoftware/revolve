
#include "core/utils.h"
#include "cpu/interface.h"
#include <cstdint>

uint32_t ProcessorInterface::read(uint32_t offset, AccessSize size) {
    if (size != AccessSize::U32) {
        throw std::runtime_error(
            "ProcessorInterface only supports word-sized reads");
    }

    switch (offset) {
    case 0x00:
        return interruptCause;
    case 0x04:
        return interruptMask;
    case 0x0C:
        return fifoBase;
    case 0x10:
        return fifoEnd;
    case 0x14:
        return fifoWritePointer;
    case 0x20:
        return errorCause;
    case 0x24:
        return errorAddress;
    case 0x28:
        return resetCode;
    case 0x2C:
        return unknown;
    case 0x30:
        return 0x246500B1;
    case 0x34:
        return busStrength;

    default:
        Logger::log("PI", LogLevel::Warning,
                    "Unimplemented PI read: 0x" + utils::toHexString(offset));
        return 0;
    }
}

void ProcessorInterface::write(uint32_t offset, uint32_t value,
                               AccessSize size) {
    if (size != AccessSize::U32) {
        throw std::runtime_error(
            "Processor Interface only supports 32-bit accesses");
    }

    switch (offset) {
    case 0x00:
        interruptCause &= ~value;
        break;

    case 0x04:
        interruptMask = value & 0x7FFFu;
        break;

    case 0x0C:
        fifoBase = value & 0x1FFFFFE0;
        break;

    case 0x10:
        fifoEnd = value & 0x1FFFFFE0;
        break;

    case 0x14:
        fifoWritePointer = value & 0x1FFFFFE0;
        break;

    case 0x18:
        break;

    case 0x20:
        errorCause = value & 0x7;
        break;

    case 0x24:
        break;

    case 0x28:
        resetCode = value;
        break;

    case 0x2C:
        unknown = value & 0x3FF;
        break;

    case 0x30:
        break;

    case 0x34:
        busStrength = value & 0x07FFFFFF;
        break;

    default:
        Logger::log("PI", LogLevel::Warning,
                    "Unknown PI write at offset 0x" +
                        utils::toHexString(offset));
        break;
    }
}
