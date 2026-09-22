
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
    const uint8_t vat = command & 0x07;
    const uint8_t primitive = command & 0xF8;

    const uint16_t vertexCount = read16();

    Logger::log("GX", LogLevel::Info,
                "Primitive: 0x" + utils::toHexString(primitive) +
                    " VAT=" + std::to_string(vat) +
                    " vertices=" + std::to_string(vertexCount));

    for (uint32_t i = 0; i < vertexCount; ++i) {
        GXVertex vertex = readVertex(vat);

        Logger::log("GX", LogLevel::Info,
                    "Vertex " + std::to_string(i) + " position=(" +
                        std::to_string(vertex.position.x) + ", " +
                        std::to_string(vertex.position.y) + ", " +
                        std::to_string(vertex.position.z) + ")");
    }
}

float GX::readComponent(GXComponentFormat format, uint8_t fractionalBits) {
    const float scale = static_cast<float>(1u << fractionalBits);

    switch (format) {
    case GXComponentFormat::U8:
        return static_cast<float>(read8()) / scale;

    case GXComponentFormat::S8:
        return static_cast<float>(static_cast<int8_t>(read8())) / scale;

    case GXComponentFormat::U16:
        return static_cast<float>(read16()) / scale;

    case GXComponentFormat::S16:
        return static_cast<float>(static_cast<int16_t>(read16())) / scale;

    case GXComponentFormat::F32: {
        uint32_t raw = read32();

        float result;
        std::memcpy(&result, &raw, sizeof(result));

        return result;
    }
    }

    Logger::log("GX", LogLevel::Warning,
                "Unknown component format: " +
                    std::to_string(static_cast<uint32_t>(format)));

    return 0.0f;
}

GXVec3 GX::readDirectPosition(uint8_t vat) {
    const GXPositionFormat fmt = state.cp.getPositionFormat(vat);

    GXVec3 result{};

    result.x = readComponent(fmt.format, fmt.fractionalBits);

    result.y = readComponent(fmt.format, fmt.fractionalBits);

    if (fmt.components == 3) {
        result.z = readComponent(fmt.format, fmt.fractionalBits);
    }

    return result;
}

GXVec3 GX::readDirectNormal(uint8_t vat) {
    const GXNormalFormat fmt = state.cp.getNormalFormat(vat);

    GXVec3 result{};

    result.x = readComponent(fmt.format, 0);
    result.y = readComponent(fmt.format, 0);
    result.z = readComponent(fmt.format, 0);

    return result;
}

GXColor GX::readDirectColor(uint8_t vat, uint32_t colorIndex) {
    GXColor result{};

    GXColorFormat format = state.cp.getColorFormat(vat, colorIndex).format;

    switch (format) {
    case GXColorFormat::RGB8:
        result.r = read8() / 255.0f;
        result.g = read8() / 255.0f;
        result.b = read8() / 255.0f;
        result.a = 1.0f;
        break;

    case GXColorFormat::RGBX8:
        result.r = read8() / 255.0f;
        result.g = read8() / 255.0f;
        result.b = read8() / 255.0f;
        read8(); // X
        result.a = 1.0f;
        break;

    case GXColorFormat::RGBA8:
        result.r = read8() / 255.0f;
        result.g = read8() / 255.0f;
        result.b = read8() / 255.0f;
        result.a = read8() / 255.0f;
        break;

    default:
        Logger::log("GX", LogLevel::Warning, "Color format not decoded yet");

        break;
    }

    return result;
}

GXVec2 GX::readDirectTexCoord(uint8_t vat, uint32_t index) {
    const GXTexCoordFormat fmt = state.cp.getTexCoordFormat(vat, index);

    GXVec2 result{};

    result.x = readComponent(fmt.format, fmt.fractionalBits);

    if (fmt.components == 2) {
        result.y = readComponent(fmt.format, fmt.fractionalBits);
    }

    return result;
}

GXVertex GX::readVertex(uint8_t vat) {
    GXVertex vertex{};

    const auto &vcd = state.cp.getVCD();

    if (vcd.positionMatrixIndex) {
        vertex.positionMatrixIndex = read8();
    }

    for (uint32_t i = 0; i < 8; ++i) {
        if (vcd.texMatrixIndex[i]) {
            vertex.texMatrixIndices[i] = read8();
        }
    }
    switch (vcd.position) {
    case GXVertexAttributeMode::None:
        break;

    case GXVertexAttributeMode::Direct:
        vertex.position = readDirectPosition(vat);
        break;

    case GXVertexAttributeMode::Index8:
    case GXVertexAttributeMode::Index16:
        Logger::log("GX", LogLevel::Warning,
                    "Indexed position not implemented yet");
        break;
    }

    switch (vcd.normal) {
    case GXVertexAttributeMode::None:
        break;

    case GXVertexAttributeMode::Direct:
        vertex.normal = readDirectNormal(vat);
        break;

    case GXVertexAttributeMode::Index8:
    case GXVertexAttributeMode::Index16:
        Logger::log("GX", LogLevel::Warning,
                    "Indexed normal not implemented yet");
        break;
    }

    if (vcd.color0 == GXVertexAttributeMode::Direct)
        vertex.color0 = readDirectColor(vat, 0);

    if (vcd.color1 == GXVertexAttributeMode::Direct)
        vertex.color1 = readDirectColor(vat, 1);

    for (uint32_t i = 0; i < 8; ++i) {
        if (vcd.texCoord[i] == GXVertexAttributeMode::Direct) {

            vertex.texCoords[i] = readDirectTexCoord(vat, i);
        }
    }

    return vertex;
}