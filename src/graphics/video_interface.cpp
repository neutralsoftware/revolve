#include "graphics/video_interface.h"
#include "core/memory.h"
#include "core/utils.h"
#include <cstdint>

uint32_t VideoInterface::read(uint32_t offset, AccessSize size) {
    switch (size) {
    case AccessSize::U16:
        return registers[offset / 2];

    case AccessSize::U32: {
        uint32_t hi = registers[offset / 2];
        uint32_t lo = registers[offset / 2 + 1];

        return (hi << 16) | lo;
    }

    default:
        Logger::log("VI", LogLevel::Error, "Invalid access size");
        return 0;
    }
}

void VideoInterface::write(uint32_t offset, uint32_t value, AccessSize size) {
    switch (size) {
    case AccessSize::U16:
        registers[offset / 2] = static_cast<uint16_t>(value);
        return;

    case AccessSize::U32:
        registers[offset / 2] = static_cast<uint16_t>(value >> 16);

        registers[offset / 2 + 1] = static_cast<uint16_t>(value);

        break;

    default:
        Logger::log("VI", LogLevel::Error, "Invalid access size");
        return;
    }
}