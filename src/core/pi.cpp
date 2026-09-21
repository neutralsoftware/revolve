
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
    case 0x14:
        // PI_FIFO_WP - implement when GX FIFO is added.
        return 0;

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
        break;

    case 0x04:
        interruptMask = value & 0x7FFFu;
        break;

    case 0x14:
        break;

    default:
        Logger::log("PI", LogLevel::Warning,
                    "Unknown PI write at offset 0x" +
                        utils::toHexString(offset));
        break;
    }
}