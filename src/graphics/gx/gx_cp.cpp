
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
        arrayStrides[reg - 0xB0] = value;
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