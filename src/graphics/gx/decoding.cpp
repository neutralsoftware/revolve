
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

    renderer->finishGXBatch();
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
    GXVec3 sa = transformToScreen(a);
    GXVec3 sb = transformToScreen(b);

    Logger::log("GX", LogLevel::Info,
                "LINE: " + std::to_string(a.position.x) + "," +
                    std::to_string(a.position.y) + " | " +
                    std::to_string(b.position.x) + "," +
                    std::to_string(b.position.y));
}

void GX::emitPoint(const GXVertex &point) {
    GXVec3 sp = transformToScreen(point);

    Logger::log("GX", LogLevel::Info,
                "POINT: " + std::to_string(point.position.x) + "," +
                    std::to_string(point.position.y));
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

    if (address < state.xf.registers.size())
        state.xf.registers[address] = value;
}

GXMatrix3x4 GX::getPositionMatrix(uint32_t matrixIndex) const {
    GXMatrix3x4 result{};

    const uint32_t base = matrixIndex;

    for (uint32_t row = 0; row < 3; ++row) {
        for (uint32_t col = 0; col < 4; ++col) {
            result.m[row][col] = state.xf.matrixMemory[base + row * 4 + col];
        }
    }

    return result;
}

uint32_t GX::getVertexPositionMatrixIndex(const GXVertex &vertex) const {

    if (state.cp.getVCD().positionMatrixIndex)
        return vertex.positionMatrixIndex;

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

    out.u = vertex.texCoords[0].x;
    out.v = vertex.texCoords[0].y;

    return out;
}

void GX::initialize() { renderer->initialize(); }
