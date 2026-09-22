
#include "core/utils.h"
#include "device.h"
#include "graphics/gx.h"
#include <cstdint>

uint8_t GX::read8() {
    if (reader.availableBytes == 0) {
        Logger::log("GX", LogLevel::Error,
                    "Attempted FIFO read with no data available");
        return 0;
    }

    uint8_t value = Bus::readPhysical8(reader.cursor);
    const auto &fifo = Device::globalDevice->cp->getFifo();

    reader.cursor++;
    reader.bytesIntoBlock++;
    reader.availableBytes--;

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
        return;
    case GXCommand::CPLoad:
        processCPLoad();
        return;
    case GXCommand::XFLoad:
        processXFLoad();
        return;
    case GXCommand::BPLoad:
        processBPLoad();
        return;
    }

    if (command & 0x80) {
        processPrimitive(command);
        return;
    }

    Logger::log("GX", LogLevel::Warning,
                "Unknown GX command: 0x" + utils::toHexString(command));
}

void GX::processCPLoad() {
    uint8_t reg = read8();
    uint32_t value = read32();

    state.cp.write(reg, value);

    Logger::log("GX", LogLevel::Info,
                "CPLoad: Register 0x" + utils::toHexString(reg) + " = 0x" +
                    utils::toHexString(value));
}

void GX::processBPLoad() {
    uint32_t raw = read32();

    uint8_t reg = static_cast<uint8_t>(raw >> 24);

    uint32_t value = raw & 0x00FFFFFF;

    state.bp.registers[reg] = value;

    Logger::log("GX", LogLevel::Info,
                "BPLoad: Register 0x" + utils::toHexString(reg) + " = 0x" +
                    utils::toHexString(value));
}

void GX::processXFLoad() {
    uint32_t header = read32();

    uint16_t address = static_cast<uint16_t>(header & 0xFFFF);

    uint32_t count = (header >> 16) + 1;

    Logger::log("GX", LogLevel::Info,
                "XF load addr=" + utils::toHexString(address) +
                    " count=" + std::to_string(count));

    for (uint32_t i = 0; i < count; i++) {
        uint32_t value = read32();

        uint32_t target = static_cast<uint32_t>(address) + i;

        if (target < state.xf.registers.size())
            state.xf.registers[target] = value;
        else {
            Logger::log("GX", LogLevel::Warning,
                        "XF address out of range: " +
                            utils::toHexString(target));
        }
    }
}

void GX::run() {
    while (reader.availableBytes > 0) {
        processCommand();
    }
}

void GX::initializeFifoReader() {
    const auto &fifo = Device::globalDevice->cp->getFifo();

    reader.cursor = fifo.readPointer;
    reader.bytesIntoBlock = 0;
    reader.availableBytes = fifo.readWriteDistance;
}

void GX::processPrimitive(uint8_t command) {
    uint8_t vat = command & 0x07;
    uint8_t primitive = command & 0xF8;

    uint16_t vertexCount = read16();

    uint32_t vertexSize = state.cp.getVertexSize(vat);

    Logger::log("GX", LogLevel::Info,
                "Primitive: 0x" + utils::toHexString(primitive) +
                    " VAT: " + std::to_string(vat) +
                    " Vertex Count: " + std::to_string(vertexCount) +
                    " Partial Vertex Size: " + std::to_string(vertexSize));
}