
#include "core/utils.h"
#include "device.h"
#include "graphics/gx.h"
#include <cmath>
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

    writeBP(reg, value);
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

        writeXF(static_cast<uint16_t>(address + i), value);
    }
}

void GX::run() {
    if (!Device::globalDevice->cp->isFifoReadEnabled())
        return;

    if (reader.availableBytes == 0)
        return;

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

    result.x = readComponent(fmt.format, 0);
    result.y = readComponent(fmt.format, 0);
    result.z = readComponent(fmt.format, 0);

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
        uint32_t index = readAttributeIndex(vcd.normal);

        vertex.normal = readIndexedNormal(vat, index);
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
    result.x = readMemoryComponent(address, fmt.format, 0);
    result.y = readMemoryComponent(address, fmt.format, 0);
    result.z = readMemoryComponent(address, fmt.format, 0);

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
        uint16_t raw = static_cast<uint16_t>(readByte()) << 8 | readByte();

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
        uint16_t raw = static_cast<uint16_t>(readByte()) << 8 | readByte();

        result.r = ((raw >> 12) & 0xF) / 15.0f;
        result.g = ((raw >> 8) & 0xF) / 15.0f;
        result.b = ((raw >> 4) & 0xF) / 15.0f;
        result.a = (raw & 0xF) / 15.0f;
        break;
    }

    case GXColorFormat::RGBA6: {
        uint32_t raw = static_cast<uint32_t>(readByte()) << 16 |
                       static_cast<uint32_t>(readByte()) << 8 | readByte();

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

        result.x = readComponent(fmt.format, 0);
        result.y = readComponent(fmt.format, 0);
        result.z = readComponent(fmt.format, 0);

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

    if (address >= 0x500 && address < 0x600) {
        float f;
        std::memcpy(&f, &value, sizeof(f));

        state.xf.postMatrices[address - 0x500] = f;
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
    out.z = clip.z;
    out.w = clip.w;

    out.r = vertex.color0.r;
    out.g = vertex.color0.g;
    out.b = vertex.color0.b;
    out.a = vertex.color0.a;

    auto singleUvParsing = [&](uint32_t index, float &u, float &v) -> void {
        GXVec3 tex = generateTexCoord(vertex, index);

        u = tex.x;
        v = tex.y;

        if (state.xf.texGens[index].projection == GXTexProjection::STQ &&
            tex.z != 0.0f) {
            u /= tex.z;
            v /= tex.z;
        }
    };

    singleUvParsing(0, out.u0, out.v0);
    singleUvParsing(1, out.u1, out.v1);
    singleUvParsing(2, out.u2, out.v2);
    singleUvParsing(3, out.u3, out.v3);
    singleUvParsing(4, out.u4, out.v4);
    singleUvParsing(5, out.u5, out.v5);
    singleUvParsing(6, out.u6, out.v6);
    singleUvParsing(7, out.u7, out.v7);

    return out;
}

void GX::initialize() { renderer->initialize(); }

void GX::writeBP(uint8_t reg, uint32_t value) {
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

        renderer->setRasterState(r);
        break;
    }
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
        state.bp.copy.xfbStride = value & 0x3FF;
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
        state.bp.copy.clearAfterCopy = (value & BP_COPY_CLEAR_MASK) != 0;
        state.bp.copy.copyToXfb = (value & BP_COPY_TO_XFB_MASK) != 0;

        executeEfbCopy();

        break;
    case 0x59: {
        renderer->flushEFB();

        state.bp.scissor.offsetXHalf = static_cast<uint16_t>(value & 0x3FF);
        state.bp.scissor.offsetYHalf =
            static_cast<uint16_t>((value >> 10) & 0x3FF);

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

    renderer->presentXFB();
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

    const uint32_t base = matrixIndex * 4;

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

GXVec3 GX::generateTexCoord(const GXVertex &vertex, uint32_t index) const {
    if (index >= state.xf.numTexGens) {
        return {};
    }

    const auto &gen = state.xf.texGens[index];

    if (gen.type != GXTexGenType::Regular) {
        return {};
    }

    GXVec4 source = getTexGenSource(vertex, gen.source);

    if (gen.inputForm == GXTexInputForm::AB11) {
        source.z = 1.0f;
    }

    uint32_t matrixIndex = getTextureMatrixIndex(vertex, index);
    GXVec3 result = applyTextureMatrix(source, matrixIndex, gen.projection);
    result = applyPostTextureMatrix(result, index);

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