
#include "graphics/wgpipe.h"
#include "core/utils.h"
#include "device.h"
#include <cstdint>

uint32_t WriteGatherPipe::read(uint32_t offset, AccessSize size) {
    Logger::log("WGPIPE", LogLevel::Warning, "Read from WGPIPE");
    return 0;
}

void WriteGatherPipe::write(uint32_t offset, uint32_t value, AccessSize size) {
    switch (size) {
    case AccessSize::U8:
        push8(static_cast<uint8_t>(value));
        break;
    case AccessSize::U16:
        push16(static_cast<uint16_t>(value));
        break;
    case AccessSize::U32:
        push32(value);
        break;
    default:
        Logger::log("WGPIPE", LogLevel::Error,
                    "Invalid access size: " +
                        std::to_string(static_cast<uint32_t>(size)));
        return;
    }
}

void WriteGatherPipe::push8(uint8_t value) {
    buffer[count++] = value;

    if (count == buffer.size()) {
        flush();
    }
}

void WriteGatherPipe::push16(uint16_t value) {
    push8(static_cast<uint8_t>(value >> 8));
    push8(static_cast<uint8_t>(value));
}

void WriteGatherPipe::push32(uint32_t value) {
    push8(static_cast<uint8_t>(value >> 24));
    push8(static_cast<uint8_t>(value >> 16));
    push8(static_cast<uint8_t>(value >> 8));
    push8(static_cast<uint8_t>(value));
}

void WriteGatherPipe::flush() {
    auto *cp = Device::globalDevice->cp.get();
    uint32_t destination = cp->getFifo().writePointer;

    for (uint32_t i = 0; i < 32; i++) {
        Bus::writePhysical8(destination + i, buffer[i]);
    }

    cp->onGatherPipeBurst();

    count = 0;
}