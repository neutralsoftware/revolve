
#include "core/utils.h"
#include "device.h"
#include "graphics/gx.h"
#include <array>
#include <cmath>
#include <cstdint>

namespace {
uint8_t normalFractionalBits(GXComponentFormat format) {
    switch (format) {
    case GXComponentFormat::U8: return 7;
    case GXComponentFormat::S8: return 6;
    case GXComponentFormat::U16: return 15;
    case GXComponentFormat::S16: return 14;
    default: return 0;
    }
}
}

uint8_t GX::read8() {
    if (commandSource == GXCommandSource::DisplayList) {
        if (displayListReader.remaining == 0) {
            Logger::log("GX", LogLevel::Error,
                        "Attempted to read past end of GX display list");

            return 0;
        }

        uint8_t value = Bus::readPhysical8(displayListReader.address);

        displayListReader.address++;
        displayListReader.remaining--;

        return value;
    }

    return fifoBuffer.at(fifoBufferOffset++);
}

bool GX::commandAvailable() const {
    const bool display = commandSource == GXCommandSource::DisplayList;
    const size_t available = display ? displayListReader.remaining
                                    : fifoBuffer.size() - fifoBufferOffset;
    if (available == 0)
        return false;
    auto peek = [&](size_t offset) -> uint8_t {
        return display ? Bus::readPhysical8(displayListReader.address + offset)
                       : fifoBuffer[fifoBufferOffset + offset];
    };
    const uint8_t command = peek(0);
    size_t required = 1;
    switch (static_cast<GXCommand>(command)) {
    case GXCommand::CPLoad:
        required = 6;
        break;
    case GXCommand::XFLoad:
        if (available < 5)
            return false;
        required = 5 + ((((peek(1) & 15u) << 8) | peek(2)) + 1) * 4;
        break;
    case GXCommand::XFIndexedLoadA:
    case GXCommand::XFIndexedLoadB:
    case GXCommand::XFIndexedLoadC:
    case GXCommand::XFIndexedLoadD:
    case GXCommand::BPLoad:
        required = 5;
        break;
    case GXCommand::CallDisplayList:
        required = 9;
        break;
    default:
        if (command & 0x80) {
            if (available < 3)
                return false;
            const uint32_t count = (static_cast<uint32_t>(peek(1)) << 8) | peek(2);
            required = 3 + static_cast<size_t>(count) * state.cp.getVertexSize(command & 7);
        }
        break;
    }
    return available >= required;
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

    case GXCommand::XFIndexedLoadA:
    case GXCommand::XFIndexedLoadB:
    case GXCommand::XFIndexedLoadC:
    case GXCommand::XFIndexedLoadD:
        processIndexedXF(command);
        return;

    case GXCommand::CallDisplayList:
        processCallDisplayList();
        return;

    case GXCommand::InvalidateVertexCache:
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
    uint32_t incoming = raw & 0x00FFFFFF;

    if (reg == 0xFE) {
        state.bp.writeMask = incoming;
        return;
    }

    uint32_t oldValue = state.bp.registers[reg];
    uint32_t mask = state.bp.writeMask;
    uint32_t value = (oldValue & ~mask) | (incoming & mask);

    state.bp.registers[reg] = value;
    state.bp.writeMask = 0x00FFFFFF;

    writeBP(reg, value);
}

void GX::processXFLoad() {
    uint32_t header = read32();

    uint16_t address = static_cast<uint16_t>(header & 0xFFFF);

    uint32_t count = ((header >> 16) & 0xFFF) + 1;

    Logger::log("GX", LogLevel::Info,
                "XF load addr=" + utils::toHexString(address) +
                    " count=" + std::to_string(count));

    for (uint32_t i = 0; i < count; i++) {
        uint32_t value = read32();

        writeXF(static_cast<uint16_t>(address + i), value);
    }
}

void GX::run() {
    if (!Device::globalDevice->cp->isFifoReadEnabled())
        return;

    auto *cp = Device::globalDevice->cp.get();
    const auto &fifo = cp->getFifo();
    while (reader.availableBytes >= 32 && cp->canReadFifo()) {
        for (uint32_t i = 0; i < 32; ++i)
            fifoBuffer.push_back(Bus::readPhysical8(reader.cursor + i));
        reader.cursor = reader.cursor == fifo.end ? fifo.base : reader.cursor + 32;
        reader.availableBytes -= 32;
        cp->onFifoBlockConsumed();
    }

    while (commandAvailable())
        processCommand();

    if (fifoBufferOffset != 0) {
        fifoBuffer.erase(fifoBuffer.begin(), fifoBuffer.begin() + fifoBufferOffset);
        fifoBufferOffset = 0;
    }
}

void GX::initializeFifoReader() {
    const auto &fifo = Device::globalDevice->cp->getFifo();

    reader.cursor = fifo.readPointer;
    reader.bytesIntoBlock = 0;
    reader.availableBytes = fifo.readWriteDistance;
}

void GX::onFifoBytesAvailable(uint32_t bytes) {
    if (reader.availableBytes == 0) {
        const auto &fifo = Device::globalDevice->cp->getFifo();
        reader.cursor = fifo.readPointer;
        reader.bytesIntoBlock = 0;
    }

    reader.availableBytes += bytes;
}

void GX::processPrimitive(uint8_t command) {
    uint8_t vat = command & 0x07;
    uint8_t primitive = command & 0xF8;

    uint16_t vertexCount = read16();

    std::vector<GXVertex> vertices;
    vertices.reserve(vertexCount);

    for (uint32_t i = 0; i < vertexCount; ++i) {
        vertices.push_back(readVertex(vat));
    }

    assemblePrimitive(static_cast<GXPrimitive>(primitive), vertices);
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

    result.x = readComponent(fmt.format, normalFractionalBits(fmt.format));
    result.y = readComponent(fmt.format, normalFractionalBits(fmt.format));
    result.z = readComponent(fmt.format, normalFractionalBits(fmt.format));

    return result;
}

GXColor GX::readDirectColor(uint8_t vat, uint32_t colorIndex) {
    GXColor result{};

    GXColorFormat format = state.cp.getColorFormat(vat, colorIndex).format;

    switch (format) {
    case GXColorFormat::RGB565: {
        uint16_t raw = read16();
        result.r = static_cast<float>((raw >> 11) & 0x1F) / 31.0f;
        result.g = static_cast<float>((raw >> 5) & 0x3F) / 63.0f;
        result.b = static_cast<float>(raw & 0x1F) / 31.0f;
        result.a = 1.0f;
        break;
    }

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

    case GXColorFormat::RGBA4: {
        uint16_t raw = read16();
        result.r = static_cast<float>((raw >> 12) & 0xF) / 15.0f;
        result.g = static_cast<float>((raw >> 8) & 0xF) / 15.0f;
        result.b = static_cast<float>((raw >> 4) & 0xF) / 15.0f;
        result.a = static_cast<float>(raw & 0xF) / 15.0f;
        break;
    }

    case GXColorFormat::RGBA6: {
        uint32_t raw = static_cast<uint32_t>(read8()) << 16;
        raw |= static_cast<uint32_t>(read8()) << 8;
        raw |= read8();
        result.r = static_cast<float>((raw >> 18) & 0x3F) / 63.0f;
        result.g = static_cast<float>((raw >> 12) & 0x3F) / 63.0f;
        result.b = static_cast<float>((raw >> 6) & 0x3F) / 63.0f;
        result.a = static_cast<float>(raw & 0x3F) / 63.0f;
        break;
    }

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
    case GXVertexAttributeMode::Index16: {
        uint32_t index = readAttributeIndex(vcd.position);

        vertex.position = readIndexedPosition(vat, index);
        break;
    }
    }

    switch (vcd.normal) {
    case GXVertexAttributeMode::None:
        break;

    case GXVertexAttributeMode::Direct: {
        const auto fmt = state.cp.getNormalFormat(vat);

        if (fmt.vectors == 1) {
            vertex.normal = readDirectNormal(vat);
        } else {
            readDirectNBT(vat, vertex);
        }

        break;
    }

    case GXVertexAttributeMode::Index8:
    case GXVertexAttributeMode::Index16: {
        const auto fmt = state.cp.getNormalFormat(vat);
        const uint32_t index = readAttributeIndex(vcd.normal);
        uint32_t address = getIndexedAddress(GXArrayAttribute::Normal, index);
        const uint8_t fraction = normalFractionalBits(fmt.format);
        auto readVector = [&]() {
            GXVec3 value{};
            value.x = readMemoryComponent(address, fmt.format, fraction);
            value.y = readMemoryComponent(address, fmt.format, fraction);
            value.z = readMemoryComponent(address, fmt.format, fraction);
            return value;
        };
        vertex.normal = readVector();
        if (fmt.vectors == 3) {
            const uint32_t vectorSize = state.cp.getDirectNormalSize(vat) / 3;
            if (fmt.index3)
                address = getIndexedAddress(GXArrayAttribute::Normal,
                                            readAttributeIndex(vcd.normal)) + vectorSize;
            vertex.binormal = readVector();
            if (fmt.index3)
                address = getIndexedAddress(GXArrayAttribute::Normal,
                                            readAttributeIndex(vcd.normal)) + vectorSize * 2;
            vertex.tangent = readVector();
        }
        break;
    }
    }

    auto readColor = [&](GXVertexAttributeMode mode, uint32_t colorIndex,
                         GXColor &destination) {
        switch (mode) {
        case GXVertexAttributeMode::None:
            break;

        case GXVertexAttributeMode::Direct:
            destination = readDirectColor(vat, colorIndex);
            break;

        case GXVertexAttributeMode::Index8:
        case GXVertexAttributeMode::Index16: {
            uint32_t index = readAttributeIndex(mode);

            destination = readIndexedColor(vat, colorIndex, index);
            break;
        }
        }
    };

    readColor(vcd.color0, 0, vertex.color0);
    readColor(vcd.color1, 1, vertex.color1);

    for (uint32_t i = 0; i < 8; ++i) {
        switch (vcd.texCoord[i]) {
        case GXVertexAttributeMode::None:
            break;

        case GXVertexAttributeMode::Direct:
            vertex.texCoords[i] = readDirectTexCoord(vat, i);
            break;

        case GXVertexAttributeMode::Index8:
        case GXVertexAttributeMode::Index16: {
            uint32_t index = readAttributeIndex(vcd.texCoord[i]);

            vertex.texCoords[i] = readIndexedTexCoord(vat, i, index);
            break;
        }
        }
    }

    return vertex;
}

uint32_t GX::readAttributeIndex(GXVertexAttributeMode mode) {
    switch (mode) {
    case GXVertexAttributeMode::Index8:
        return read8();

    case GXVertexAttributeMode::Index16:
        return read16();

    default:
        return 0;
    }
}

uint32_t GX::getIndexedAddress(GXArrayAttribute attribute,
                               uint32_t index) const {
    const uint32_t slot = static_cast<uint32_t>(attribute) - 9;

    const uint32_t base = state.cp.getArrayBase(slot);

    const uint32_t stride = state.cp.getArrayStride(slot);

    return base + index * stride;
}

float GX::readMemoryComponent(uint32_t &address, GXComponentFormat format,
                              uint8_t fractionalBits) {
    const float scale = static_cast<float>(1u << fractionalBits);

    switch (format) {
    case GXComponentFormat::U8:
        return static_cast<float>(readMemory8(address)) / scale;

    case GXComponentFormat::S8:
        return static_cast<float>(static_cast<int8_t>(readMemory8(address))) /
               scale;

    case GXComponentFormat::U16:
        return static_cast<float>(readMemory16(address)) / scale;

    case GXComponentFormat::S16:
        return static_cast<float>(static_cast<int16_t>(readMemory16(address))) /
               scale;

    case GXComponentFormat::F32: {
        uint32_t raw = readMemory32(address);

        float result;
        std::memcpy(&result, &raw, sizeof(result));

        return result;
    }
    }

    return 0.0f;
}

GXVec3 GX::readIndexedPosition(uint8_t vat, uint32_t index) {
    uint32_t address = getIndexedAddress(GXArrayAttribute::Position, index);

    const GXPositionFormat fmt = state.cp.getPositionFormat(vat);

    GXVec3 result{};
    result.x = readMemoryComponent(address, fmt.format, fmt.fractionalBits);
    result.y = readMemoryComponent(address, fmt.format, fmt.fractionalBits);

    if (fmt.components == 3) {
        result.z = readMemoryComponent(address, fmt.format, fmt.fractionalBits);
    }

    return result;
}

GXVec3 GX::readIndexedNormal(uint8_t vat, uint32_t index) {
    uint32_t address = getIndexedAddress(GXArrayAttribute::Normal, index);

    const GXNormalFormat fmt = state.cp.getNormalFormat(vat);

    GXVec3 result{};
    result.x = readMemoryComponent(address, fmt.format, normalFractionalBits(fmt.format));
    result.y = readMemoryComponent(address, fmt.format, normalFractionalBits(fmt.format));
    result.z = readMemoryComponent(address, fmt.format, normalFractionalBits(fmt.format));

    return result;
}

GXVec2 GX::readIndexedTexCoord(uint8_t vat, uint32_t texIndex, uint32_t index) {
    const auto attribute = static_cast<GXArrayAttribute>(
        static_cast<uint8_t>(GXArrayAttribute::Tex0) + texIndex);

    uint32_t address = getIndexedAddress(attribute, index);

    const GXTexCoordFormat fmt = state.cp.getTexCoordFormat(vat, texIndex);

    GXVec2 result{};
    result.x = readMemoryComponent(address, fmt.format, fmt.fractionalBits);

    if (fmt.components == 2) {
        result.y = readMemoryComponent(address, fmt.format, fmt.fractionalBits);
    }

    return result;
}

GXColor GX::readIndexedColor(uint8_t vat, uint32_t colorIndex, uint32_t index) {
    GXArrayAttribute attribute =
        colorIndex == 0 ? GXArrayAttribute::Color0 : GXArrayAttribute::Color1;

    uint32_t address = getIndexedAddress(attribute, index);

    GXColorFormat format = state.cp.getColorFormat(vat, colorIndex).format;

    GXColor result{};

    auto readByte = [&]() { return Bus::readPhysical8(address++); };

    switch (format) {
    case GXColorFormat::RGB8:
        result.r = readByte() / 255.0f;
        result.g = readByte() / 255.0f;
        result.b = readByte() / 255.0f;
        result.a = 1.0f;
        break;

    case GXColorFormat::RGBX8:
        result.r = readByte() / 255.0f;
        result.g = readByte() / 255.0f;
        result.b = readByte() / 255.0f;
        readByte();
        result.a = 1.0f;
        break;

    case GXColorFormat::RGBA8:
        result.r = readByte() / 255.0f;
        result.g = readByte() / 255.0f;
        result.b = readByte() / 255.0f;
        result.a = readByte() / 255.0f;
        break;

    case GXColorFormat::RGB565: {
        uint16_t raw = static_cast<uint16_t>(readByte()) << 8;
        raw |= readByte();

        uint32_t r = (raw >> 11) & 0x1F;
        uint32_t g = (raw >> 5) & 0x3F;
        uint32_t b = raw & 0x1F;

        result.r = r / 31.0f;
        result.g = g / 63.0f;
        result.b = b / 31.0f;
        result.a = 1.0f;
        break;
    }

    case GXColorFormat::RGBA4: {
        uint16_t raw = static_cast<uint16_t>(readByte()) << 8;
        raw |= readByte();

        result.r = ((raw >> 12) & 0xF) / 15.0f;
        result.g = ((raw >> 8) & 0xF) / 15.0f;
        result.b = ((raw >> 4) & 0xF) / 15.0f;
        result.a = (raw & 0xF) / 15.0f;
        break;
    }

    case GXColorFormat::RGBA6: {
        uint32_t raw = static_cast<uint32_t>(readByte()) << 16;
        raw |= static_cast<uint32_t>(readByte()) << 8;
        raw |= readByte();

        result.r = ((raw >> 18) & 0x3F) / 63.0f;
        result.g = ((raw >> 12) & 0x3F) / 63.0f;
        result.b = ((raw >> 6) & 0x3F) / 63.0f;
        result.a = (raw & 0x3F) / 63.0f;
        break;
    }

    default:
        Logger::log("GX", LogLevel::Warning,
                    "Indexed packed color not implemented yet");
        break;
    }

    return result;
}

void GX::readDirectNBT(uint8_t vat, GXVertex &vertex) {
    const auto fmt = state.cp.getNormalFormat(vat);

    auto readVec = [&]() {
        GXVec3 result{};

        result.x = readComponent(fmt.format, normalFractionalBits(fmt.format));
        result.y = readComponent(fmt.format, normalFractionalBits(fmt.format));
        result.z = readComponent(fmt.format, normalFractionalBits(fmt.format));

        return result;
    };

    vertex.normal = readVec();
    vertex.binormal = readVec();
    vertex.tangent = readVec();
}

void GX::emitTriangle(const GXVertex &a, const GXVertex &b, const GXVertex &c) {
    renderer->drawTriangle(transformToRenderVertex(a),
                           transformToRenderVertex(b),
                           transformToRenderVertex(c));
}

void GX::emitLine(const GXVertex &a, const GXVertex &b) {
    GXRenderVertex ra = transformToRenderVertex(a);
    GXRenderVertex rb = transformToRenderVertex(b);
    if (ra.w == 0.0f || rb.w == 0.0f)
        return;

    const float ax = ra.x / ra.w;
    const float ay = ra.y / ra.w;
    const float bx = rb.x / rb.w;
    const float by = rb.y / rb.w;
    const float dx = bx - ax;
    const float dy = by - ay;
    const float length = std::sqrt(dx * dx + dy * dy);
    if (length == 0.0f) {
        emitPoint(a);
        return;
    }

    const float nx = -dy / length * 0.008f;
    const float ny = dx / length * 0.008f;
    GXRenderVertex a0 = ra;
    GXRenderVertex a1 = ra;
    GXRenderVertex b0 = rb;
    GXRenderVertex b1 = rb;
    a0.x += nx * ra.w;
    a0.y += ny * ra.w;
    a1.x -= nx * ra.w;
    a1.y -= ny * ra.w;
    b0.x += nx * rb.w;
    b0.y += ny * rb.w;
    b1.x -= nx * rb.w;
    b1.y -= ny * rb.w;
    renderer->drawTriangle(a0, a1, b1);
    renderer->drawTriangle(a0, b1, b0);
}

void GX::emitPoint(const GXVertex &point) {
    GXRenderVertex center = transformToRenderVertex(point);
    if (center.w == 0.0f)
        return;

    constexpr float halfSize = 0.015f;
    GXRenderVertex bottomLeft = center;
    GXRenderVertex bottomRight = center;
    GXRenderVertex topLeft = center;
    GXRenderVertex topRight = center;
    bottomLeft.x -= halfSize * center.w;
    bottomLeft.y -= halfSize * center.w;
    bottomRight.x += halfSize * center.w;
    bottomRight.y -= halfSize * center.w;
    topLeft.x -= halfSize * center.w;
    topLeft.y += halfSize * center.w;
    topRight.x += halfSize * center.w;
    topRight.y += halfSize * center.w;
    renderer->drawTriangle(bottomLeft, bottomRight, topRight);
    renderer->drawTriangle(bottomLeft, topRight, topLeft);
}

void GX::assemblePrimitive(GXPrimitive primitive,
                           const std::vector<GXVertex> &vertices) {
    switch (primitive) {
    case GXPrimitive::Triangles:
        for (size_t i = 0; i + 2 < vertices.size(); i += 3) {
            emitTriangle(vertices[i], vertices[i + 1], vertices[i + 2]);
        }
        break;
    case GXPrimitive::TriangleStrip:
        for (size_t i = 2; i < vertices.size(); ++i) {
            if ((i & 1) == 0) {
                emitTriangle(vertices[i - 2], vertices[i - 1], vertices[i]);
            } else {
                emitTriangle(vertices[i - 1], vertices[i - 2], vertices[i]);
            }
        }
        break;
    case GXPrimitive::TriangleFan:
        for (size_t i = 2; i < vertices.size(); ++i) {
            emitTriangle(vertices[0], vertices[i - 1], vertices[i]);
        }
        break;
    case GXPrimitive::Quads:
        for (size_t i = 0; i + 3 < vertices.size(); i += 4) {
            emitTriangle(vertices[i], vertices[i + 1], vertices[i + 2]);

            emitTriangle(vertices[i], vertices[i + 2], vertices[i + 3]);
        }
        break;
    case GXPrimitive::Lines:
        for (size_t i = 0; i + 1 < vertices.size(); i += 2) {
            emitLine(vertices[i], vertices[i + 1]);
        }
        break;
    case GXPrimitive::LineStrip:
        for (size_t i = 1; i < vertices.size(); ++i) {
            emitLine(vertices[i - 1], vertices[i]);
        }
        break;
    case GXPrimitive::Points:
        for (const GXVertex &vertex : vertices) {
            emitPoint(vertex);
        }
        break;
    }
}

void GX::writeXF(uint16_t address, uint32_t value) {
    if (address < state.xf.matrixMemory.size()) {
        float f;
        std::memcpy(&f, &value, sizeof(f));

        state.xf.matrixMemory[address] = f;
        return;
    }

    if (address >= 0x400 && address < 0x460) {
        float f;
        std::memcpy(&f, &value, sizeof(f));

        state.xf.normalMatrixMemory[address - 0x400] = f;
        return;
    }

    if (address >= 0x500 && address < 0x600) {
        float f;
        std::memcpy(&f, &value, sizeof(f));

        state.xf.postMatrices[address - 0x500] = f;
        return;
    }

    if (address >= 0x600 && address < 0x680) {
        const uint32_t relative = address - 0x600;

        const uint32_t lightIndex = relative / 0x10;
        const uint32_t word = relative % 0x10;

        if (lightIndex >= state.xf.lights.size())
            return;

        GXLight &light = state.xf.lights[lightIndex];

        auto rawFloat = [&]() {
            float f;
            std::memcpy(&f, &value, sizeof(f));
            return f;
        };

        switch (word) {
        case 0:
        case 1:
        case 2:
            break;

        case 3:
            light.color.r = static_cast<float>((value >> 24) & 0xFF) / 255.0f;
            light.color.g = static_cast<float>((value >> 16) & 0xFF) / 255.0f;
            light.color.b = static_cast<float>((value >> 8) & 0xFF) / 255.0f;
            light.color.a = static_cast<float>(value & 0xFF) / 255.0f;
            break;

        case 4:
            light.cosAttenuation.x = rawFloat();
            break;
        case 5:
            light.cosAttenuation.y = rawFloat();
            break;
        case 6:
            light.cosAttenuation.z = rawFloat();
            break;

        case 7:
            light.distAttenuation.x = rawFloat();
            break;
        case 8:
            light.distAttenuation.y = rawFloat();
            break;
        case 9:
            light.distAttenuation.z = rawFloat();
            break;

        case 0xA:
            light.position.x = rawFloat();
            break;
        case 0xB:
            light.position.y = rawFloat();
            break;
        case 0xC:
            light.position.z = rawFloat();
            break;

        case 0xD:
            light.direction.x = rawFloat();
            break;
        case 0xE:
            light.direction.y = rawFloat();
            break;
        case 0xF:
            light.direction.z = rawFloat();
            break;
        }

        return;
    }

    if (address == 0x1009) {
        state.xf.numColorChannels = static_cast<uint8_t>(value & 0x3);

        return;
    }

    if (address == 0x100A) {
        state.xf.ambientColors[0] = gx::decodeXFColor(value);
        return;
    }

    if (address == 0x100B) {
        state.xf.ambientColors[1] = gx::decodeXFColor(value);
        return;
    }

    if (address == 0x100C) {
        state.xf.materialColors[0] = gx::decodeXFColor(value);
        return;
    }

    if (address == 0x100D) {
        state.xf.materialColors[1] = gx::decodeXFColor(value);
        return;
    }

    if (address == 0x100E) {
        state.xf.lightingChannels[0].colorControl = value;
        return;
    }

    if (address == 0x100F) {
        state.xf.lightingChannels[1].colorControl = value;
        return;
    }

    if (address == 0x1010) {
        state.xf.lightingChannels[0].alphaControl = value;
        return;
    }

    if (address == 0x1011) {
        state.xf.lightingChannels[1].alphaControl = value;
        return;
    }

    if (address >= 0x1020 && address <= 0x1025) {
        float f;
        std::memcpy(&f, &value, sizeof(f));

        state.xf.projection.values[address - 0x1020] = f;
        return;
    }

    if (address == 0x1026) {
        state.xf.projection.mode = value;
        return;
    }

    if (address >= 0x101A && address <= 0x101F) {
        float f;
        std::memcpy(&f, &value, sizeof(f));

        switch (address) {
        case 0x101A:
            state.xf.viewport.xScale = f;
            break;

        case 0x101B:
            state.xf.viewport.yScale = f;
            break;

        case 0x101C:
            state.xf.viewport.zRange = f;
            break;

        case 0x101D:
            state.xf.viewport.xOrigin = f;
            break;

        case 0x101E:
            state.xf.viewport.yOrigin = f;
            break;

        case 0x101F:
            state.xf.viewport.farZ = f;
            break;
        }

        return;
    }

    if (address == 0x1012) {
        state.xf.dualTexTransform = (value & 1) != 0;
        return;
    }

    if (address == 0x103F) {
        state.xf.numTexGens = static_cast<uint8_t>(value & 0xFu);
        return;
    }

    if (address >= 0x1040 && address <= 0x1047) {
        uint32_t index = address - 0x1040;

        auto &gen = state.xf.texGens[index];

        gen.projection = static_cast<GXTexProjection>((value >> 1) & 0x1);
        gen.inputForm = static_cast<GXTexInputForm>((value >> 2) & 0x1);
        gen.type = static_cast<GXTexGenType>((value >> 4) & 0x7);
        gen.source = static_cast<GXTexSource>((value >> 7) & 0x1F);
        gen.embossSource = static_cast<uint8_t>((value >> 12) & 0x7);
        gen.embossLight = static_cast<uint8_t>((value >> 15) & 0x7);

        return;
    }

    if (address >= 0x1050 && address <= 0x1057) {
        uint32_t index = address - 0x1050;

        auto &post = state.xf.postTexMatrices[index];
        post.index = static_cast<uint8_t>(value & 0x3F);
        post.normalize = ((value >> 8) & 1) != 0;

        return;
    }

    if (address < state.xf.registers.size())
        state.xf.registers[address] = value;
}

GXMatrix3x4 GX::getPositionMatrix(uint32_t matrixIndex) const {
    GXMatrix3x4 result{};

    const uint32_t base = matrixIndex * 4;

    if (base + 11 >= state.xf.matrixMemory.size())
        return result;

    for (uint32_t row = 0; row < 3; ++row) {
        for (uint32_t col = 0; col < 4; ++col) {
            result.m[row][col] = state.xf.matrixMemory[base + row * 4 + col];
        }
    }

    return result;
}

uint32_t GX::getVertexPositionMatrixIndex(const GXVertex &vertex) const {
    if (state.cp.getVCD().positionMatrixIndex)
        return vertex.positionMatrixIndex & 0x3F;

    return state.cp.getPositionMatrixIndex();
}

GXVec4 GX::transformPosition(const GXVertex &vertex) const {

    uint32_t matrixIndex = getVertexPositionMatrixIndex(vertex);

    GXMatrix3x4 matrix = getPositionMatrix(matrixIndex);

    const GXVec3 &p = vertex.position;

    GXVec4 out{};

    out.x = matrix.m[0][0] * p.x + matrix.m[0][1] * p.y + matrix.m[0][2] * p.z +
            matrix.m[0][3];

    out.y = matrix.m[1][0] * p.x + matrix.m[1][1] * p.y + matrix.m[1][2] * p.z +
            matrix.m[1][3];

    out.z = matrix.m[2][0] * p.x + matrix.m[2][1] * p.y + matrix.m[2][2] * p.z +
            matrix.m[2][3];

    out.w = 1.0f;

    return out;
}

GXVec4 GX::projectPosition(const GXVec4 &v) const {
    const auto &p = state.xf.projection.values;

    GXVec4 out{};

    if (state.xf.projection.mode == 0) {
        // Perspective
        out.x = p[0] * v.x + p[1] * v.z;
        out.y = p[2] * v.y + p[3] * v.z;
        out.z = p[4] * v.z + p[5];
        out.w = -v.z;
    } else {
        // Orthographic
        out.x = p[0] * v.x + p[1];
        out.y = p[2] * v.y + p[3];
        out.z = p[4] * v.z + p[5];
        out.w = 1.0f;
    }

    return out;
}

GXVec3 GX::clipToNDC(const GXVec4 &clip) const {
    if (clip.w == 0.0f)
        return {};

    return {clip.x / clip.w, clip.y / clip.w, clip.z / clip.w};
}

GXVec3 GX::viewportTransform(const GXVec3 &ndc) const {
    const auto &vp = state.xf.viewport;

    GXVec3 result{};
    result.x = ndc.x * vp.xScale + vp.xOrigin;
    result.y = ndc.y * vp.yScale + vp.yOrigin;
    result.z = ndc.z * vp.zRange + vp.farZ;

    return result;
}

GXVec3 GX::transformToScreen(const GXVertex &vertex) const {
    GXVec4 view = transformPosition(vertex);
    GXVec4 clip = projectPosition(view);
    GXVec3 ndc = clipToNDC(clip);

    return viewportTransform(ndc);
}

GXRenderVertex GX::transformToRenderVertex(const GXVertex &vertex) const {
    GXVec4 view = transformPosition(vertex);
    GXVec4 clip = projectPosition(view);

    GXRenderVertex out{};

    out.x = clip.x;
    out.y = clip.y;
#ifdef OPENGL
    out.z = clip.z;
#else
    out.z = (clip.z + clip.w) * 0.5f;
#endif
    out.w = clip.w;

    out.r = vertex.color0.r;
    out.g = vertex.color0.g;
    out.b = vertex.color0.b;
    out.a = vertex.color0.a;

    const uint32_t matrixIndex = getVertexPositionMatrixIndex(vertex);

    const GXVec3 transformedNormal = transformNormal(vertex, matrixIndex);

    const GXVec3 viewPosition{view.x, view.y, view.z};

    GXColor color0{};
    GXColor color1{};

    if (state.xf.numColorChannels >= 1) {
        color0 = calculateLightingChannel(vertex, 0, viewPosition,
                                          transformedNormal);
    } else {
        color0 = getVertexColor(vertex, 0);
    }

    if (state.xf.numColorChannels >= 2) {
        color1 = calculateLightingChannel(vertex, 1, viewPosition,
                                          transformedNormal);
    } else {
        color1 = color0;
    }

    out.r = color0.r;
    out.g = color0.g;
    out.b = color0.b;
    out.a = color0.a;

    out.r1 = color1.r;
    out.g1 = color1.g;
    out.b1 = color1.b;
    out.a1 = color1.a;

    std::array<GXVec3, 8> generatedTexCoords{};

    auto singleUvParsing = [&](uint32_t index, float &u, float &v, float &q,
                               const std::array<GXVec3, 8> &generated) -> void {
        GXVec3 tex = generateTexCoord(vertex, index, generated);

        u = tex.x;
        v = tex.y;
        q = tex.z;

    };

    singleUvParsing(0, out.u0, out.v0, out.q0, generatedTexCoords);
    singleUvParsing(1, out.u1, out.v1, out.q1, generatedTexCoords);
    singleUvParsing(2, out.u2, out.v2, out.q2, generatedTexCoords);
    singleUvParsing(3, out.u3, out.v3, out.q3, generatedTexCoords);
    singleUvParsing(4, out.u4, out.v4, out.q4, generatedTexCoords);
    singleUvParsing(5, out.u5, out.v5, out.q5, generatedTexCoords);
    singleUvParsing(6, out.u6, out.v6, out.q6, generatedTexCoords);
    singleUvParsing(7, out.u7, out.v7, out.q7, generatedTexCoords);

    return out;
}

void GX::initialize() { renderer->initialize(); }

void GX::writeBP(uint8_t reg, uint32_t value) {
    if (reg == 0x45 && (value & 0xFF) == 2) {
        renderer->flushEFB();
        Device::globalDevice->pe->finish();
        return;
    }
    if (reg == 0x47 || reg == 0x48) {
        renderer->flushEFB();
        Device::globalDevice->pe->setToken(static_cast<uint16_t>(value), reg == 0x48);
        return;
    }
    if (int unit = gx::textureUnitFromBP(reg, 0x80, 0xA0); unit >= 0) {
        renderer->flushEFB();
        decodeTextureMode0(static_cast<uint32_t>(unit), value);
        updateTextureUnit(unit);
        return;
    }

    // TEX MODE 1 / LOD
    if (int unit = gx::textureUnitFromBP(reg, 0x84, 0xA4); unit >= 0) {
        return;
    }

    if (int unit = gx::textureUnitFromBP(reg, 0x88, 0xA8); unit >= 0) {
        renderer->flushEFB();
        decodeTextureImage0(static_cast<uint32_t>(unit), value);
        updateTextureUnit(unit);
        return;
    }

    if (int unit = gx::textureUnitFromBP(reg, 0x8C, 0xAC); unit >= 0) {
        decodeTextureImage1(unit, value);
        return;
    }

    if (int unit = gx::textureUnitFromBP(reg, 0x90, 0xB0); unit >= 0) {
        decodeTextureImage2(unit, value);
        return;
    }

    if (int unit = gx::textureUnitFromBP(reg, 0x94, 0xB4); unit >= 0) {
        renderer->flushEFB();
        decodeTextureImage3(static_cast<uint32_t>(unit), value);
        updateTextureUnit(unit);
        return;
    }

    if (int unit = gx::textureUnitFromBP(reg, 0x98, 0xB8); unit >= 0) {
        renderer->flushEFB();
        decodeTextureTLUT(static_cast<uint32_t>(unit), value);
        updateTextureUnit(unit);
        return;
    }

    if (reg >= 0x06 && reg <= 0x0E) {
        renderer->flushEFB();
        decodeIndirectMatrixWord(reg, value);
        return;
    }

    if (reg >= 0x10 && reg <= 0x1F) {
        renderer->flushEFB();

        decodeTevIndirect(reg - 0x10, value);

        return;
    }

    if (reg == 0x42) {
        renderer->flushEFB();

        state.bp.destinationAlpha.alpha = static_cast<uint8_t>(value & 0xFF);
        state.bp.destinationAlpha.enabled = ((value >> 8) & 1) != 0;

        return;
    }

    if (reg >= 0xC0 && reg <= 0xDF) {
        renderer->flushEFB();

        uint32_t stage = (reg - 0xC0) / 2;
        if ((reg & 1u) == 0) {
            decodeTevColorCombiner(stage, value);
        } else {
            decodeTevAlphaCombiner(stage, value);
        }
        return;
    }

    if (reg >= 0xE0 && reg <= 0xE7) {
        renderer->flushEFB();

        decodeTevRegister(reg, value);

        return;
    }

    if (reg >= 0xF6 && reg <= 0xFD) {
        renderer->flushEFB();

        decodeTevKSel(reg, value);

        return;
    }

    switch (reg) {
    case 0x00: {
        renderer->flushEFB();

        uint32_t cull = (value >> 14) & 0x3;

        switch (cull) {
        case 0:
            state.bp.raster.cullMode = opal::CullMode::None;
            break;
        case 1:
            state.bp.raster.cullMode = opal::CullMode::Back;
            break;
        case 2:
            state.bp.raster.cullMode = opal::CullMode::Front;
            break;
        case 3:
            state.bp.raster.cullMode = opal::CullMode::FrontAndBack;
            break;
        }

        state.bp.tevStageCount =
            static_cast<uint8_t>(((value >> 10) & 0xFu) + 1);

        state.bp.indirectStageCount = static_cast<uint8_t>((value >> 16) & 0x7);
        state.bp.indirectStageCount =
            std::min<uint8_t>(state.bp.indirectStageCount, 4);

        renderer->setRasterState(state.bp.raster);

        break;
    }
    case 0x20: {
        renderer->flushEFB();

        state.bp.scissor.top = static_cast<uint16_t>(value & 0x7FF);
        state.bp.scissor.left = static_cast<uint16_t>((value >> 12) & 0x7FF);

        updateScissorState();

        break;
    }
    case 0x21: {
        renderer->flushEFB();

        state.bp.scissor.bottom = static_cast<uint16_t>(value & 0x7FF);
        state.bp.scissor.right = static_cast<uint16_t>((value >> 12) & 0x7FF);

        updateScissorState();

        break;
    }
    case 0x25:
    case 0x26:
        renderer->flushEFB();
        decodeIndirectScale(reg, value);
        break;
    case 0x27:
        renderer->flushEFB();
        decodeIndirectRef(value);
        break;
    case 0x28:
    case 0x29:
    case 0x2A:
    case 0x2B:
    case 0x2C:
    case 0x2D:
    case 0x2E:
    case 0x2F:
        renderer->flushEFB();
        decodeTevOrder(reg, value);
        break;
    case 0x40: {
        renderer->flushEFB();

        state.bp.raster.depthTest = (value & 1) != 0;

        uint32_t compare = (value >> 1) & 0x7;

        switch (compare) {
        case 0:
            state.bp.raster.depthCompare = opal::CompareOp::Never;
            break;

        case 1:
            state.bp.raster.depthCompare = opal::CompareOp::Less;
            break;

        case 2:
            state.bp.raster.depthCompare = opal::CompareOp::Equal;
            break;

        case 3:
            state.bp.raster.depthCompare = opal::CompareOp::LessEqual;
            break;

        case 4:
            state.bp.raster.depthCompare = opal::CompareOp::Greater;
            break;

        case 5:
            state.bp.raster.depthCompare = opal::CompareOp::NotEqual;
            break;

        case 6:
            state.bp.raster.depthCompare = opal::CompareOp::GreaterEqual;
            break;

        case 7:
            state.bp.raster.depthCompare = opal::CompareOp::Always;
            break;
        }

        state.bp.raster.depthWrite = (value & (1u << 4)) != 0;

        renderer->setRasterState(state.bp.raster);

        break;
    }
    case 0x41: {
        renderer->flushEFB();

        auto &r = state.bp.raster;
        r.blendEnabled = (value & (1u << 0)) != 0;
        r.logicOpEnabled = (value & (1u << 1)) != 0;
        r.colorWrite = (value & (1u << 3)) != 0;
        r.alphaWrite = (value & (1u << 4)) != 0;

        uint32_t dstFactor = (value >> 5) & 0x7;

        uint32_t srcFactor = (value >> 8) & 0x7;

        r.subtractBlend = (value & (1u << 11)) != 0;
        r.logicOp = static_cast<uint8_t>((value >> 12) & 0xF);
        r.srcBlend = decodeGXSrcBlendFactor(srcFactor);
        r.dstBlend = decodeGXDstBlendFactor(dstFactor);

        r.dither = (value & (1u << 2)) != 0;

        renderer->setRasterState(r);
        break;
    }
    case 0x43:
        renderer->flushEFB();
        state.bp.raster.pixelFormat = static_cast<uint8_t>(value & 0x7);
        state.bp.raster.zCompareBeforeTexture = (value & (1u << 6)) != 0;
        renderer->setRasterState(state.bp.raster);
        break;
    case 0x49:
        state.bp.copy.sourceX = value & 0x3FF;
        state.bp.copy.sourceY = (value >> 10) & 0x3FF;
        break;
    case 0x4A:
        state.bp.copy.sourceWidth = (value & 0x3FF) + 1;
        state.bp.copy.sourceHeight = ((value >> 10) & 0x3FF) + 1;
        break;
    case 0x4B:
        state.bp.copy.xfbAddress = (value & 0x00FFFFFF) << 5;
        break;
    case 0x4D:
        state.bp.copy.xfbStride = (value & 0x3FF) << 5;
        break;
    case 0x4E:
        state.bp.copy.yScale = value & 0x1FF;
        break;
    case 0x4F: {
        uint8_t a = static_cast<uint8_t>(value & 0xFF);

        uint8_t r = static_cast<uint8_t>((value >> 8) & 0xFF);

        state.bp.copy.clearColor.a = a / 255.0f;

        state.bp.copy.clearColor.r = r / 255.0f;

        break;
    }

    case 0x50: {
        uint8_t g = static_cast<uint8_t>((value >> 8) & 0xFF);

        uint8_t b = static_cast<uint8_t>(value & 0xFF);

        state.bp.copy.clearColor.g = g / 255.0f;

        state.bp.copy.clearColor.b = b / 255.0f;

        break;
    }

    case 0x51:
        state.bp.copy.clearDepth = value & 0xFFFFFF;
        break;

    case 0x52:
        state.bp.copy.clampTop = (value & (1u << 0)) != 0;
        state.bp.copy.clampBottom = (value & (1u << 1)) != 0;
        state.bp.copy.gamma = static_cast<uint8_t>((value >> 7) & 0x3);
        state.bp.copy.scaleInverted = (value & (1u << 10)) != 0;
        state.bp.copy.clearAfterCopy = (value & BP_COPY_CLEAR_MASK) != 0;
        state.bp.copy.frameToField = static_cast<uint8_t>((value >> 12) & 0x3);
        state.bp.copy.copyToXfb = (value & BP_COPY_TO_XFB_MASK) != 0;

        executeEfbCopy();

        break;
    case 0x59: {
        renderer->flushEFB();

        state.bp.scissor.offsetXHalf = static_cast<uint16_t>(value & 0x1FF);
        state.bp.scissor.offsetYHalf =
            static_cast<uint16_t>((value >> 10) & 0x1FF);

        updateScissorState();

        break;
    }
    case 0xF3: {
        renderer->flushEFB();

        auto &a = state.bp.alphaTest;
        a.ref0 = static_cast<uint8_t>(value & 0xFF);
        a.ref1 = static_cast<uint8_t>((value >> 8) & 0xFF);
        a.comp0 = decodeGXCompare((value >> 16) & 0x7);
        a.comp1 = decodeGXCompare((value >> 19) & 0x7);
        a.logic = static_cast<GXAlphaLogic>((value >> 22) & 0x3);

        renderer->setAlphaTestState(a);

        break;
    }
    case 0xE8:
        renderer->flushEFB();
        state.bp.fog.rangeCenter = static_cast<uint16_t>(value & 0x3FF);
        state.bp.fog.rangeAdjustmentEnabled = (value & (1u << 10)) != 0;
        break;
    case 0xE9:
    case 0xEA:
    case 0xEB:
    case 0xEC:
    case 0xED: {
        renderer->flushEFB();
        const size_t index = static_cast<size_t>(reg - 0xE9) * 2;
        state.bp.fog.rangeK[index] = static_cast<uint16_t>(value & 0xFFF);
        state.bp.fog.rangeK[index + 1] =
            static_cast<uint16_t>((value >> 12) & 0xFFF);
        break;
    }
    case 0xEE:
        renderer->flushEFB();
        state.bp.fog.a = gx::decodeFogFloat(value);
        break;
    case 0xEF:
        renderer->flushEFB();
        state.bp.fog.bMagnitude = value & 0xFFFFFF;
        break;
    case 0xF0:
        renderer->flushEFB();
        state.bp.fog.bShift = static_cast<uint8_t>(value & 0x1F);
        break;
    case 0xF1: {
        renderer->flushEFB();

        const uint32_t type = ((value >> 21) & 0x7) |
                              (((value >> 20) & 0x1) << 3);

        state.bp.fog.type = static_cast<GXFogType>(type);
        state.bp.fog.enabled = type != 0;

        state.bp.fog.c = gx::decodeFogFloat(value);
        break;
    }
    case 0xF2: {
        renderer->flushEFB();

        state.bp.fog.color.r =
            static_cast<float>((value >> 16) & 0xFF) / 255.0f;
        state.bp.fog.color.g = static_cast<float>((value >> 8) & 0xFF) / 255.0f;
        state.bp.fog.color.b = static_cast<float>(value & 0xFF) / 255.0f;
        state.bp.fog.color.a = 1.0f;

        return;
    }
    case 0xF4: {
        renderer->flushEFB();

        state.bp.zTexture.bias = value & 0x00FFFFFF;

        break;
    }
    case 0xF5: {
        renderer->flushEFB();

        state.bp.zTexture.format = static_cast<GXZTextureFormat>(value & 0x3);
        state.bp.zTexture.op = static_cast<GXZTextureOp>((value >> 2) & 0x3);

        if (static_cast<uint8_t>(state.bp.zTexture.op) > 2) {
            state.bp.zTexture.op = GXZTextureOp::Disabled;
        }

        break;
    }
    }
}

void GX::executeEfbCopy() {
    renderer->flushEFB();

    if (!state.bp.copy.copyToXfb) {
        Logger::log("GX", LogLevel::Warning,
                    "Texture EFB copies not implemented yet");
        return;
    }

    renderer->copyEFBToXFB(state.bp.copy);

    if (state.bp.copy.clearAfterCopy) {
        renderer->clearEFB(state.bp.copy.clearColor, state.bp.copy.clearDepth);
    }

}

void GX::updateScissorState() {
    const auto &s = state.bp.scissor;

    const int32_t offsetX = static_cast<int32_t>(s.offsetXHalf) * 2;
    const int32_t offsetY = static_cast<int32_t>(s.offsetYHalf) * 2;

    int32_t left = static_cast<int32_t>(s.left) - offsetX;
    int32_t top = static_cast<int32_t>(s.top) - offsetY;
    int32_t right = static_cast<int32_t>(s.right) - offsetX;
    int32_t bottom = static_cast<int32_t>(s.bottom) - offsetY;

    int32_t width = right - left + 1;
    int32_t height = bottom - top + 1;

    int32_t x0 = std::clamp<int32_t>(left, 0, EFB_WIDTH);
    int32_t y0 = std::clamp<int32_t>(top, 0, EFB_HEIGHT);
    int32_t x1 = std::clamp<int32_t>(left + width, 0, EFB_WIDTH);
    int32_t y1 = std::clamp<int32_t>(top + height, 0, EFB_HEIGHT);

    state.bp.raster.scissorX = static_cast<uint16_t>(x0);
    state.bp.raster.scissorY = static_cast<uint16_t>(y0);
    state.bp.raster.scissorWidth = static_cast<uint16_t>(std::max(0, x1 - x0));
    state.bp.raster.scissorHeight = static_cast<uint16_t>(std::max(0, y1 - y0));

    renderer->setRasterState(state.bp.raster);
}

opal::BlendFunc GX::decodeGXSrcBlendFactor(uint32_t factor) const {
    switch (factor) {
    case 0:
        return opal::BlendFunc::Zero;
    case 1:
        return opal::BlendFunc::One;
    case 2:
        return opal::BlendFunc::DstColor;
    case 3:
        return opal::BlendFunc::OneMinusDstColor;
    case 4:
        return opal::BlendFunc::SrcAlpha;
    case 5:
        return opal::BlendFunc::OneMinusSrcAlpha;
    case 6:
        return opal::BlendFunc::DstAlpha;
    case 7:
        return opal::BlendFunc::OneMinusDstAlpha;
    }

    return opal::BlendFunc::One;
}

opal::BlendFunc GX::decodeGXDstBlendFactor(uint32_t factor) const {
    switch (factor) {
    case 0:
        return opal::BlendFunc::Zero;
    case 1:
        return opal::BlendFunc::One;
    case 2:
        return opal::BlendFunc::SrcColor;
    case 3:
        return opal::BlendFunc::OneMinusSrcColor;
    case 4:
        return opal::BlendFunc::SrcAlpha;
    case 5:
        return opal::BlendFunc::OneMinusSrcAlpha;
    case 6:
        return opal::BlendFunc::DstAlpha;
    case 7:
        return opal::BlendFunc::OneMinusDstAlpha;
    }

    return opal::BlendFunc::Zero;
}

opal::CompareOp GX::decodeGXCompare(uint32_t value) const {
    switch (value) {
    case 0:
        return opal::CompareOp::Never;
    case 1:
        return opal::CompareOp::Less;
    case 2:
        return opal::CompareOp::Equal;
    case 3:
        return opal::CompareOp::LessEqual;
    case 4:
        return opal::CompareOp::Greater;
    case 5:
        return opal::CompareOp::NotEqual;
    case 6:
        return opal::CompareOp::GreaterEqual;
    case 7:
        return opal::CompareOp::Always;
    }

    return opal::CompareOp::Always;
}

uint32_t GX::getTextureMatrixIndex(const GXVertex &vertex,
                                   uint32_t texGen) const {
    if (texGen >= 8)
        return 0;

    if (state.cp.getVCD().texMatrixIndex[texGen]) {
        return vertex.texMatrixIndices[texGen] & 0x3F;
    }

    return state.cp.getTextureMatrixIndex(texGen);
}

GXVec4 GX::getTexGenSource(const GXVertex &vertex, GXTexSource source) const {
    switch (source) {
    case GXTexSource::Position:
        return {vertex.position.x, vertex.position.y, vertex.position.z, 1.0f};
    case GXTexSource::Normal:
        return {vertex.normal.x, vertex.normal.y, vertex.normal.z, 1.0f};
    case GXTexSource::Tex0:
    case GXTexSource::Tex1:
    case GXTexSource::Tex2:
    case GXTexSource::Tex3:
    case GXTexSource::Tex4:
    case GXTexSource::Tex5:
    case GXTexSource::Tex6:
    case GXTexSource::Tex7: {
        uint32_t tex = static_cast<uint32_t>(source) -
                       static_cast<uint32_t>(GXTexSource::Tex0);

        return {vertex.texCoords[tex].x, vertex.texCoords[tex].y, 1.0f, 1.0f};
    }

    default: {
        return {
            0.0f,
            0.0f,
            0.0f,
            1.0f,
        };
    }
    }
}

GXVec3 GX::applyTextureMatrix(const GXVec4 &v, uint32_t matrixIndex,
                              GXTexProjection projection) const {

    GXVec3 out{};

    const uint32_t base = (matrixIndex & 0x3F) * 4;

    auto dotRow = [&](uint32_t row) -> float {
        const uint32_t offset = base + row * 4;

        return state.xf.matrixMemory[offset + 0] * v.x +
               state.xf.matrixMemory[offset + 1] * v.y +
               state.xf.matrixMemory[offset + 2] * v.z +
               state.xf.matrixMemory[offset + 3] * v.w;
    };

    out.x = dotRow(0);
    out.y = dotRow(1);

    if (projection == GXTexProjection::STQ)
        out.z = dotRow(2);
    else
        out.z = 1.0f;

    return out;
}

GXVec3 GX::generateTexCoord(const GXVertex &vertex, uint32_t index,
                            const std::array<GXVec3, 8> &generated) const {
    if (index >= state.xf.numTexGens)
        return {};

    const GXTexGenState &gen = state.xf.texGens[index];

    if (gen.type == GXTexGenType::EmbossMap) {
        if (gen.embossSource >= 8 || gen.embossLight >= 8) {
            return {};
        }

        const GXVec3 base = generated[gen.embossSource];

        const uint32_t matrixIndex = getVertexPositionMatrixIndex(vertex);

        GXMatrix3x3 normalMatrix = getNormalMatrix(matrixIndex);

        auto transformDirection = [&](const GXVec3 &v) -> GXVec3 {
            return {
                normalMatrix.m[0][0] * v.x + normalMatrix.m[0][1] * v.y +
                    normalMatrix.m[0][2] * v.z,

                normalMatrix.m[1][0] * v.x + normalMatrix.m[1][1] * v.y +
                    normalMatrix.m[1][2] * v.z,

                normalMatrix.m[2][0] * v.x + normalMatrix.m[2][1] * v.y +
                    normalMatrix.m[2][2] * v.z,
            };
        };

        GXVec3 tangent = transformDirection(vertex.tangent);
        GXVec3 binormal = transformDirection(vertex.binormal);

        const GXVec4 view = transformPosition(vertex);

        GXVec3 lightDir{state.xf.lights[gen.embossLight].position.x - view.x,
                        state.xf.lights[gen.embossLight].position.y - view.y,
                        state.xf.lights[gen.embossLight].position.z - view.z};

        lightDir = gx::normalize(lightDir);

        return {base.x + gx::dot(lightDir, tangent),
                base.y + gx::dot(lightDir, binormal), 1.0f};
    }

    switch (gen.type) {
    case GXTexGenType::Color0: {
        const GXVec4 view = transformPosition(vertex);

        const uint32_t matrixIndex = getVertexPositionMatrixIndex(vertex);

        const GXVec3 normal = transformNormal(vertex, matrixIndex);

        GXColor color = calculateLightingChannel(
            vertex, 0, {view.x, view.y, view.z}, normal);

        return {color.r, color.g, 1.0f};
    }

    case GXTexGenType::Color1: {
        const GXVec4 view = transformPosition(vertex);

        const uint32_t matrixIndex = getVertexPositionMatrixIndex(vertex);

        const GXVec3 normal = transformNormal(vertex, matrixIndex);

        GXColor color = calculateLightingChannel(
            vertex, 1, {view.x, view.y, view.z}, normal);

        return {color.r, color.g, 1.0f};
    }

    default:
        break;
    }

    GXVec4 source = getTexGenSource(vertex, gen.source);

    if (gen.inputForm == GXTexInputForm::AB11) {
        source.z = 1.0f;
    }

    uint32_t matrixIndex = getTextureMatrixIndex(vertex, index);
    GXVec3 result = applyTextureMatrix(source, matrixIndex, gen.projection);

    if (state.xf.dualTexTransform) {
        const GXPostTexMatrixState &post = state.xf.postTexMatrices[index];

        if (post.normalize) {
            const float len =
                std::sqrt(result.x * result.x + result.y * result.y +
                          result.z * result.z);

            if (len > 0.0f) {
                result.x /= len;
                result.y /= len;
                result.z /= len;
            }
        }

        result = applyPostTextureMatrix(result, index);
    }

    return result;
}

GXVec3 GX::applyPostTextureMatrix(GXVec3 tex, uint32_t texGen) const {
    if (!state.xf.dualTexTransform)
        return tex;

    const auto &post = state.xf.postTexMatrices[texGen];

    if (post.normalize) {
        float len = std::sqrt(tex.x * tex.x + tex.y * tex.y + tex.z * tex.z);

        if (len != 0.0f) {
            tex.x /= len;
            tex.y /= len;
            tex.z /= len;
        }
    }

    const uint32_t base = post.index;

    GXVec3 out{};

    auto row = [&](uint32_t r) -> float {
        const uint32_t i = (base + r) & 0x3F;

        return state.xf.postMatrices[i * 4 + 0] * tex.x +
               state.xf.postMatrices[i * 4 + 1] * tex.y +
               state.xf.postMatrices[i * 4 + 2] * tex.z +
               state.xf.postMatrices[i * 4 + 3];
    };

    out.x = row(0);
    out.y = row(1);
    out.z = row(2);

    return out;
}

void GX::processIndexedXF(uint8_t command) {
    const uint16_t index = read16();
    const uint16_t control = read16();

    const uint32_t xfAddress = control & 0x0FFF;

    const uint32_t length = ((control >> 12) & 0xF) + 1;

    uint32_t arrayIndex;

    switch (command) {
    case static_cast<uint8_t>(GXCommand::XFIndexedLoadA):
        arrayIndex = 12;
        break;

    case static_cast<uint8_t>(GXCommand::XFIndexedLoadB):
        arrayIndex = 13;
        break;

    case static_cast<uint8_t>(GXCommand::XFIndexedLoadC):
        arrayIndex = 14;
        break;

    case static_cast<uint8_t>(GXCommand::XFIndexedLoadD):
        arrayIndex = 15;
        break;

    default:
        Logger::log("GX", LogLevel::Error, "Invalid indexed XF command");

        return;
    }

    const uint32_t base = state.cp.getArrayBase(arrayIndex);
    const uint32_t stride = state.cp.getArrayStride(arrayIndex);

    uint32_t sourceAddress = base + static_cast<uint32_t>(index) * stride;

    for (uint32_t i = 0; i < length; ++i) {
        uint32_t value = readMemory32(sourceAddress);

        writeXF(static_cast<uint16_t>(xfAddress + i), value);
    }
}

void GX::processCallDisplayList() {
    const uint32_t address = read32();
    const uint32_t size = read32();

    if (size == 0)
        return;

    processDisplayList(address, size);
}

void GX::processDisplayList(uint32_t address, uint32_t size) {
    constexpr uint32_t MAX_DISPLAY_LIST_DEPTH = 32;

    if (displayListDepth >= MAX_DISPLAY_LIST_DEPTH) {
        Logger::log("GX", LogLevel::Error,
                    "GX display-list recursion limit reached");

        return;
    }

    const GXCommandSource previousSource = commandSource;
    const GXDisplayListReader previousReader = displayListReader;

    commandSource = GXCommandSource::DisplayList;

    displayListReader.address = address;
    displayListReader.remaining = size;

    displayListDepth++;

    while (commandAvailable())
        processCommand();
    if (displayListReader.remaining != 0)
        Logger::log("GX", LogLevel::Error, "Truncated GX display list");

    displayListDepth--;

    commandSource = previousSource;
    displayListReader = previousReader;
}

GXMatrix3x3 GX::getNormalMatrix(uint32_t matrixIndex) const {
    GXMatrix3x3 result{};

    const uint32_t row = matrixIndex & 0x1F;
    const uint32_t base = row * 3;

    if (base + 8 >= state.xf.normalMatrixMemory.size())
        return result;

    for (uint32_t r = 0; r < 3; ++r) {
        for (uint32_t c = 0; c < 3; ++c) {
            result.m[r][c] = state.xf.normalMatrixMemory[base + r * 3 + c];
        }
    }

    return result;
}

GXVec3 GX::transformNormal(const GXVertex &vertex, uint32_t matrixIndex) const {
    const GXMatrix3x3 matrix = getNormalMatrix(matrixIndex);

    GXVec3 result{};

    result.x = matrix.m[0][0] * vertex.normal.x +
               matrix.m[0][1] * vertex.normal.y +
               matrix.m[0][2] * vertex.normal.z;

    result.y = matrix.m[1][0] * vertex.normal.x +
               matrix.m[1][1] * vertex.normal.y +
               matrix.m[1][2] * vertex.normal.z;

    result.z = matrix.m[2][0] * vertex.normal.x +
               matrix.m[2][1] * vertex.normal.y +
               matrix.m[2][2] * vertex.normal.z;

    const float length = std::sqrt(result.x * result.x + result.y * result.y +
                                   result.z * result.z);

    if (length > 0.0f) {
        result.x /= length;
        result.y /= length;
        result.z /= length;
    }

    return result;
}
