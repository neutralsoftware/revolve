
#include "device.h"
#include "graphics/gx.h"
#include <cstdint>

uint8_t GX::read8() {
    uint8_t value = Bus::readPhysical8(reader.cursor);
    auto &fifo = Device::globalDevice->cp->getFifo();

    reader.cursor++;
    reader.bytesIntoBlock++;

    if (reader.cursor > fifo.end)
        reader.cursor = fifo.base;

    if (reader.bytesIntoBlock == 32) {
        reader.bytesIntoBlock = 0;
        Device::globalDevice->cp->onFifoBlockConsumed();
    }

    return value;
}

uint16_t GX::read16() {
    uint16_t hi = static_cast<uint16_t>(read8()) << 8;
    uint16_t lo = static_cast<uint16_t>(read8());

    return hi | lo;
}

uint32_t GX::read32() {
    uint32_t hi = static_cast<uint32_t>(read16()) << 16;
    uint32_t lo = static_cast<uint32_t>(read16());

    return hi | lo;
}

void GX::processCommand() {
    uint8_t command = read8();
    switch (static_cast<GXCommand>(command)) {
    case GXCommand::NOP:
        break;
    case GXCommand::CPLoad:
        break;
    case GXCommand::XFLoad:
        break;
    case GXCommand::BPLoad:
        break;
    }
}