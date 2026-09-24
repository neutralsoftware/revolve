
#include "graphics/gx.h"

void GXCommandProcessorState::write(uint8_t reg, uint32_t value) {
    registers[reg] = value;

    if (reg == 0x50) {
        decodeVCDLo(value);
        return;
    }

    if (reg == 0x60) {
        decodeVCDHi(value);
        return;
    }

    if (reg == 0x30) {
        decodeMatrixIndexA(value);
        return;
    }

    if (reg == 0x40) {
        decodeMatrixIndexB(value);
        return;
    }

    if (reg >= 0x70 && reg <= 0x77) {
        vat[reg - 0x70].a = value;
        return;
    }

    if (reg >= 0x80 && reg <= 0x87) {
        vat[reg - 0x80].b = value;
        return;
    }

    if (reg >= 0x90 && reg <= 0x97) {
        vat[reg - 0x90].c = value;
        return;
    }

    if (reg >= 0xA0 && reg <= 0xAF) {
        arrayBases[reg - 0xA0] = value;
        return;
    }

    if (reg >= 0xB0 && reg <= 0xBF) {
        arrayStrides[reg - 0xB0] = value & 0xFF;
        return;
    }
}

void GXCommandProcessorState::decodeVCDLo(uint32_t value) {
    vcd.positionMatrixIndex = (value & (1 << 0)) != 0;

    for (uint32_t i = 0; i < 8; i++) {
        vcd.texMatrixIndex[i] = (value & (1u << (1 + i))) != 0;
    }

    vcd.position = decodeMode((value >> 9) & 0x3);

    vcd.normal = decodeMode((value >> 11) & 0x3);

    vcd.color0 = decodeMode((value >> 13) & 0x3);

    vcd.color1 = decodeMode((value >> 15) & 0x3);
}

void GXCommandProcessorState::decodeVCDHi(uint32_t value) {
    for (uint32_t i = 0; i < 8; i++) {
        vcd.texCoord[i] = decodeMode((value >> (i * 2)) & 0x3);
    }
}

GXPositionFormat
GXCommandProcessorState::getPositionFormat(uint8_t vatIndex) const {
    if (vatIndex >= vat.size())
        return {};
    const uint32_t a = vat[vatIndex].a;

    GXPositionFormat result{};
    result.components = ((a >> 0) & 0x1) ? 3 : 2;
    result.format = static_cast<GXComponentFormat>((a >> 1) & 0x7);
    result.fractionalBits = static_cast<uint8_t>((a >> 4) & 0x1F);

    return result;
}

uint32_t
GXCommandProcessorState::getDirectPositionSize(uint8_t vatIndex) const {
    auto format = getPositionFormat(vatIndex);

    return format.components * componentSize(format.format);
}

uint32_t GXCommandProcessorState::getVertexSize(uint8_t vatIndex) const {
    uint32_t size = 0;

    if (vcd.positionMatrixIndex)
        size += 1;

    for (bool enabled : vcd.texMatrixIndex) {
        if (enabled)
            size += 1;
    }

    size +=
        getAttributeIndexSize(vcd.position, getDirectPositionSize(vatIndex));

    const auto normal = getNormalFormat(vatIndex);
    const uint32_t normalIndices = normal.vectors == 3 && normal.index3 ? 3 : 1;
    size += vcd.normal == GXVertexAttributeMode::Direct
                ? getDirectNormalSize(vatIndex)
                : normalIndices * getAttributeIndexSize(vcd.normal, 0);

    size += getAttributeIndexSize(vcd.color0, getDirectColorSize(vatIndex, 0));

    size += getAttributeIndexSize(vcd.color1, getDirectColorSize(vatIndex, 1));

    for (uint32_t i = 0; i < 8; i++) {
        size += getAttributeIndexSize(vcd.texCoord[i],
                                      getDirectTexCoordSize(vatIndex, i));
    }

    return size;
}

GXNormalFormat
GXCommandProcessorState::getNormalFormat(uint8_t vatIndex) const {
    GXNormalFormat result{};

    if (vatIndex >= vat.size())
        return result;

    const uint32_t a = vat[vatIndex].a;

    result.vectors = ((a >> 9) & 0x1) ? 3 : 1;
    result.format = static_cast<GXComponentFormat>((a >> 10) & 0x7);
    result.index3 = ((a >> 31) & 0x1) != 0;

    return result;
}

uint32_t GXCommandProcessorState::getDirectNormalSize(uint8_t vatIndex) const {
    auto fmt = getNormalFormat(vatIndex);

    return fmt.vectors * 3 * componentSize(fmt.format);
}

GXColorAttributeFormat
GXCommandProcessorState::getColorFormat(uint8_t vatIndex,
                                        uint32_t colorIndex) const {
    GXColorAttributeFormat result{};

    if (vatIndex >= vat.size() || colorIndex > 1)
        return result;

    const uint32_t a = vat[vatIndex].a;

    if (colorIndex == 0) {
        result.format = static_cast<GXColorFormat>((a >> 14) & 0x7);
    } else {
        result.format = static_cast<GXColorFormat>((a >> 18) & 0x7);
    }

    return result;
}

uint32_t
GXCommandProcessorState::getDirectColorSize(uint8_t vatIndex,
                                            uint32_t colorIndex) const {
    return colorSize(getColorFormat(vatIndex, colorIndex).format);
}

GXTexCoordFormat
GXCommandProcessorState::getTexCoordFormat(uint8_t vatIndex,
                                           uint32_t texIndex) const {
    GXTexCoordFormat result{};

    if (vatIndex >= vat.size() || texIndex >= 8)
        return result;

    const GXVAT &v = vat[vatIndex];

    auto decode = [&](uint32_t word, uint32_t elementsShift,
                      uint32_t formatShift, uint32_t fracShift) {
        result.components = ((word >> elementsShift) & 0x1) ? 2 : 1;

        result.format =
            static_cast<GXComponentFormat>((word >> formatShift) & 0x7);

        result.fractionalBits =
            static_cast<uint8_t>((word >> fracShift) & 0x1F);
    };

    switch (texIndex) {
    case 0:
        decode(v.a, 21, 22, 25);
        break;

    case 1:
        decode(v.b, 0, 1, 4);
        break;

    case 2:
        decode(v.b, 9, 10, 13);
        break;

    case 3:
        decode(v.b, 18, 19, 22);
        break;

    case 4: {
        uint32_t tex4 = ((v.b >> 27) & 0xF) | ((v.c & 0x1F) << 4);

        decode(tex4, 0, 1, 4);
        break;
    }

    case 5:
        decode(v.c, 5, 6, 9);
        break;

    case 6:
        decode(v.c, 14, 15, 18);
        break;

    case 7:
        decode(v.c, 23, 24, 27);
        break;
    }

    return result;
}

uint32_t
GXCommandProcessorState::getDirectTexCoordSize(uint8_t vatIndex,
                                               uint32_t texIndex) const {

    auto fmt = getTexCoordFormat(vatIndex, texIndex);

    return fmt.components * componentSize(fmt.format);
}

void GXCommandProcessorState::decodeMatrixIndexA(uint32_t value) {
    positionMatrixIndex = static_cast<uint8_t>(value & 0x3F);

    textureMatrixIndices[0] = static_cast<uint8_t>((value >> 6) & 0x3F);
    textureMatrixIndices[1] = static_cast<uint8_t>((value >> 12) & 0x3F);
    textureMatrixIndices[2] = static_cast<uint8_t>((value >> 18) & 0x3F);
    textureMatrixIndices[3] = static_cast<uint8_t>((value >> 24) & 0x3F);
}

void GXCommandProcessorState::decodeMatrixIndexB(uint32_t value) {
    textureMatrixIndices[4] = static_cast<uint8_t>((value >> 0) & 0x3F);
    textureMatrixIndices[5] = static_cast<uint8_t>((value >> 6) & 0x3F);
    textureMatrixIndices[6] = static_cast<uint8_t>((value >> 12) & 0x3F);
    textureMatrixIndices[7] = static_cast<uint8_t>((value >> 18) & 0x3F);
}