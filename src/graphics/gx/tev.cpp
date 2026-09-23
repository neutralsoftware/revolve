
#include "core/memory.h"
#include "graphics/gx.h"
#include <AvailabilityMacros.h>
#include <cstdint>
#include <sys/wait.h>

GXDecodedTexture GX::decodeTexture(const GXTextureState &state) const {
    switch (state.format) {
    case GXTextureFormat::I4:
        return decodeTextureI4(state.address, state.width, state.height);
    case GXTextureFormat::I8:
        return decodeTextureI8(state.address, state.width, state.height);
    case GXTextureFormat::IA4:
        return decodeTextureIA4(state.address, state.width, state.height);
    case GXTextureFormat::IA8:
        return decodeTextureIA8(state.address, state.width, state.height);
    case GXTextureFormat::RGB565:
        return decodeTextureRGB565(state.address, state.width, state.height);
    case GXTextureFormat::RGB5A3:
        return decodeTextureRGB5A3(state.address, state.width, state.height);
    case GXTextureFormat::RGBA8:
        return decodeTextureRGBA8(state.address, state.width, state.height);
    case GXTextureFormat::C4:
        return decodeTextureC4(state.address, state.width, state.height,
                               state.tlut);
    case GXTextureFormat::C8:
        return decodeTextureC8(state.address, state.width, state.height,
                               state.tlut);
    case GXTextureFormat::C14X2:
        return decodeTextureC14X2(state.address, state.width, state.height,
                                  state.tlut);
    case GXTextureFormat::CMPR:
        return decodeTextureCMPR(state.address, state.width, state.height);
    default:
        return {};
    }
}

GXDecodedTexture GX::decodeTextureI4(uint32_t address, uint16_t width,
                                     uint16_t height) const {
    GXDecodedTexture result{};
    result.width = width;
    result.height = height;
    result.rgba.resize(static_cast<size_t>(width) * height * 4);

    for (uint32_t blockY = 0; blockY < height; blockY += 8) {
        for (uint32_t blockX = 0; blockX < width; blockX += 8) {
            for (uint32_t y = 0; y < 8; ++y) {
                for (uint32_t x = 0; x < 8; x += 2) {
                    uint8_t raw = Bus::readPhysical8(address++);
                    uint8_t i0 = expand4(raw >> 4);
                    uint8_t i1 = expand4(raw & 0xF);

                    writePixel(result, blockX + x, blockY + y, i0, i0, i0, i0);

                    writePixel(result, blockX + x + 1, blockY + y, i1, i1, i1,
                               i1);
                }
            }
        }
    }

    return result;
}

GXDecodedTexture GX::decodeTextureI8(uint32_t address, uint16_t width,
                                     uint16_t height) const {
    GXDecodedTexture result{};
    result.width = width;
    result.height = height;
    result.rgba.resize(static_cast<size_t>(width) * height * 4);

    for (uint32_t blockY = 0; blockY < height; blockY += 4) {
        for (uint32_t blockX = 0; blockX < width; blockX += 8) {
            for (uint32_t y = 0; y < 4; ++y) {
                for (uint32_t x = 0; x < 8; ++x) {
                    uint8_t i = Bus::readPhysical8(address++);

                    writePixel(result, blockX + x, blockY + y, i, i, i, i);
                }
            }
        }
    }

    return result;
}

GXDecodedTexture GX::decodeTextureIA4(uint32_t address, uint16_t width,
                                      uint16_t height) const {
    GXDecodedTexture result{};
    result.width = width;
    result.height = height;
    result.rgba.resize(static_cast<size_t>(width) * height * 4);

    for (uint32_t blockY = 0; blockY < height; blockY += 4) {
        for (uint32_t blockX = 0; blockX < width; blockX += 8) {
            for (uint32_t y = 0; y < 4; ++y) {
                for (uint32_t x = 0; x < 8; ++x) {
                    uint8_t raw = Bus::readPhysical8(address++);

                    uint8_t a = expand4(raw >> 4);
                    uint8_t i = expand4(raw & 0xF);

                    writePixel(result, blockX + x, blockY + y, i, i, i, a);
                }
            }
        }
    }

    return result;
}

GXDecodedTexture GX::decodeTextureIA8(uint32_t address, uint16_t width,
                                      uint16_t height) const {
    GXDecodedTexture result{};
    result.width = width;
    result.height = height;
    result.rgba.resize(static_cast<size_t>(width) * height * 4);

    for (uint32_t blockY = 0; blockY < height; blockY += 4) {
        for (uint32_t blockX = 0; blockX < width; blockX += 4) {
            for (uint32_t y = 0; y < 4; ++y) {
                for (uint32_t x = 0; x < 4; ++x) {
                    uint8_t intensity = Bus::readPhysical8(address++);
                    uint8_t alpha = Bus::readPhysical8(address++);

                    writePixel(result, blockX + x, blockY + y, intensity,
                               intensity, intensity, alpha);
                }
            }
        }
    }

    return result;
}

GXDecodedTexture GX::decodeTextureRGB565(uint32_t address, uint16_t width,
                                         uint16_t height) const {
    GXDecodedTexture result{};
    result.width = width;
    result.height = height;
    result.rgba.resize(static_cast<size_t>(width) * height * 4);

    for (uint32_t blockY = 0; blockY < height; blockY += 4) {
        for (uint32_t blockX = 0; blockX < width; blockX += 4) {
            for (uint32_t y = 0; y < 4; ++y) {
                for (uint32_t x = 0; x < 4; ++x) {
                    uint16_t raw =
                        (static_cast<uint16_t>(Bus::readPhysical8(address))
                         << 8) |
                        Bus::readPhysical8(address + 1);

                    address += 2;

                    uint8_t r = expand5((raw >> 11) & 0x1F);
                    uint8_t g = expand6((raw >> 5) & 0x3F);
                    uint8_t b = expand5(raw & 0x1F);

                    writePixel(result, blockX + x, blockY + y, r, g, b, 255);
                }
            }
        }
    }

    return result;
}
GXDecodedTexture GX::decodeTextureRGB5A3(uint32_t address, uint16_t width,
                                         uint16_t height) const {
    GXDecodedTexture result{};
    result.width = width;
    result.height = height;
    result.rgba.resize(static_cast<size_t>(width) * height * 4);

    for (uint32_t blockY = 0; blockY < height; blockY += 4) {
        for (uint32_t blockX = 0; blockX < width; blockX += 4) {
            for (uint32_t y = 0; y < 4; ++y) {
                for (uint32_t x = 0; x < 4; ++x) {

                    uint16_t raw =
                        (static_cast<uint16_t>(Bus::readPhysical8(address))
                         << 8) |
                        Bus::readPhysical8(address + 1);

                    address += 2;

                    uint8_t r, g, b, a;

                    if (raw & 0x8000) {
                        r = expand5((raw >> 10) & 0x1F);
                        g = expand5((raw >> 5) & 0x1F);
                        b = expand5(raw & 0x1F);
                        a = 255;
                    } else {
                        a = expand3((raw >> 12) & 0x7);
                        r = expand4((raw >> 8) & 0xF);
                        g = expand4((raw >> 4) & 0xF);
                        b = expand4(raw & 0xF);
                    }

                    writePixel(result, blockX + x, blockY + y, r, g, b, a);
                }
            }
        }
    }

    return result;
}

GXDecodedTexture GX::decodeTextureRGBA8(uint32_t address, uint16_t width,
                                        uint16_t height) const {
    GXDecodedTexture result{};
    result.width = width;
    result.height = height;
    result.rgba.resize(static_cast<size_t>(width) * height * 4);

    for (uint32_t blockY = 0; blockY < height; blockY += 4) {
        for (uint32_t blockX = 0; blockX < width; blockX += 4) {
            uint32_t arAddress = address;
            uint32_t gbAddress = address + 32;

            for (uint32_t y = 0; y < 4; ++y) {
                for (uint32_t x = 0; x < 4; ++x) {
                    const uint32_t pixel = y * 4 + x;

                    uint8_t a = Bus::readPhysical8(arAddress + pixel * 2);
                    uint8_t r = Bus::readPhysical8(arAddress + pixel * 2 + 1);
                    uint8_t g = Bus::readPhysical8(gbAddress + pixel * 2);
                    uint8_t b = Bus::readPhysical8(gbAddress + pixel * 2 + 1);

                    writePixel(result, blockX + x, blockY + y, r, g, b, a);
                }
            }

            address += 64;
        }
    }

    return result;
}

GXColor GX::decodeTLUTEntry(uint32_t address, GXTLUTFormat format) const {
    uint16_t raw = (static_cast<uint16_t>(Bus::readPhysical8(address)) << 8) |
                   Bus::readPhysical8(address + 1);

    GXColor out{};

    switch (format) {
    case GXTLUTFormat::IA8: {
        uint8_t intensity = static_cast<uint8_t>(raw >> 8);
        uint8_t alpha = static_cast<uint8_t>(raw & 0xFF);

        out.r = out.g = out.b = intensity / 255.0f;
        out.a = alpha / 255.0f;
        break;
    }

    case GXTLUTFormat::RGB565: {
        out.r = expand5((raw >> 11) & 0x1F) / 255.0f;
        out.g = expand6((raw >> 5) & 0x3F) / 255.0f;
        out.b = expand5(raw & 0x1F) / 255.0f;
        out.a = 1.0f;
        break;
    }

    case GXTLUTFormat::RGB5A3: {
        if (raw & 0x8000) {
            out.r = expand5((raw >> 10) & 0x1F) / 255.0f;
            out.g = expand5((raw >> 5) & 0x1F) / 255.0f;
            out.b = expand5(raw & 0x1F) / 255.0f;
            out.a = 1.0f;
        } else {
            out.a = expand3((raw >> 12) & 0x7) / 255.0f;
            out.r = expand4((raw >> 8) & 0xF) / 255.0f;
            out.g = expand4((raw >> 4) & 0xF) / 255.0f;
            out.b = expand4(raw & 0xF) / 255.0f;
        }

        break;
    }
    }

    return out;
}

GXDecodedTexture GX::decodeTextureC4(uint32_t address, uint16_t width,
                                     uint16_t height,
                                     const GXTLUTState &tlut) const {
    GXDecodedTexture result{};
    result.width = width;
    result.height = height;
    result.rgba.resize(static_cast<size_t>(width) * height * 4);

    for (uint32_t blockY = 0; blockY < height; blockY += 8) {
        for (uint32_t blockX = 0; blockX < width; blockX += 8) {
            for (uint32_t y = 0; y < 8; ++y) {
                for (uint32_t x = 0; x < 8; x += 2) {

                    uint8_t raw = Bus::readPhysical8(address++);

                    uint8_t index0 = raw >> 4;
                    uint8_t index1 = raw & 0xF;

                    GXColor c0 =
                        decodeTLUTEntry(tlut.address + index0 * 2, tlut.format);
                    GXColor c1 =
                        decodeTLUTEntry(tlut.address + index1 * 2, tlut.format);

                    writeGXColor(result, blockX + x, blockY + y, c0);
                    writeGXColor(result, blockX + x + 1, blockY + y, c1);
                }
            }
        }
    }

    return result;
}

GXDecodedTexture GX::decodeTextureC8(uint32_t address, uint16_t width,
                                     uint16_t height,
                                     const GXTLUTState &tlut) const {
    GXDecodedTexture result{};
    result.width = width;
    result.height = height;
    result.rgba.resize(static_cast<size_t>(width) * height * 4);

    for (uint32_t blockY = 0; blockY < height; blockY += 4) {
        for (uint32_t blockX = 0; blockX < width; blockX += 8) {
            for (uint32_t y = 0; y < 4; ++y) {
                for (uint32_t x = 0; x < 8; ++x) {
                    uint8_t index = Bus::readPhysical8(address++);

                    GXColor color =
                        decodeTLUTEntry(tlut.address + index * 2, tlut.format);
                    writeGXColor(result, blockX + x, blockY + y, color);
                }
            }
        }
    }

    return result;
}

GXDecodedTexture GX::decodeTextureC14X2(uint32_t address, uint16_t width,
                                        uint16_t height,
                                        const GXTLUTState &tlut) const {
    GXDecodedTexture result{};
    result.width = width;
    result.height = height;
    result.rgba.resize(static_cast<size_t>(width) * height * 4);

    for (uint32_t blockY = 0; blockY < height; blockY += 4) {
        for (uint32_t blockX = 0; blockX < width; blockX += 4) {
            for (uint32_t y = 0; y < 4; ++y) {
                for (uint32_t x = 0; x < 4; ++x) {
                    uint16_t raw =
                        (static_cast<uint16_t>(Bus::readPhysical8(address))
                         << 8) |
                        Bus::readPhysical8(address + 1);

                    address += 2;

                    uint16_t index = raw & 0x3FFF;

                    GXColor color = decodeTLUTEntry(
                        tlut.address + static_cast<uint32_t>(index) * 2,
                        tlut.format);
                    writeGXColor(result, blockX + x, blockY + y, color);
                }
            }
        }
    }

    return result;
}

struct RGBA8Pixel {
    uint8_t r, g, b, a;
};

static RGBA8Pixel decode565Pixel(uint16_t raw) {
    return {GX::expand5((raw >> 11) & 0x1F),

            GX::expand6((raw >> 5) & 0x3F),

            GX::expand5(raw & 0x1F),

            255};
}

void GX::decodeCMPRSubBlock(GXDecodedTexture &result, uint32_t &address,
                            uint32_t originX, uint32_t originY) const {
    uint16_t c0 = (static_cast<uint16_t>(Bus::readPhysical8(address)) << 8) |
                  Bus::readPhysical8(address + 1);
    uint16_t c1 =
        (static_cast<uint16_t>(Bus::readPhysical8(address + 2)) << 8) |
        Bus::readPhysical8(address + 3);

    address += 4;

    RGBA8Pixel colors[4];

    colors[0] = decode565Pixel(c0);
    colors[1] = decode565Pixel(c1);

    if (c0 > c1) {
        colors[2] = {static_cast<uint8_t>((2 * colors[0].r + colors[1].r) / 3),
                     static_cast<uint8_t>((2 * colors[0].g + colors[1].g) / 3),
                     static_cast<uint8_t>((2 * colors[0].b + colors[1].b) / 3),
                     255};
        colors[3] = {static_cast<uint8_t>((colors[0].r + 2 * colors[1].r) / 3),
                     static_cast<uint8_t>((colors[0].g + 2 * colors[1].g) / 3),
                     static_cast<uint8_t>((colors[0].b + 2 * colors[1].b) / 3),
                     255};
    } else {
        uint8_t r = static_cast<uint8_t>((colors[0].r + colors[1].r) / 2);
        uint8_t g = static_cast<uint8_t>((colors[0].g + colors[1].g) / 2);
        uint8_t b = static_cast<uint8_t>((colors[0].b + colors[1].b) / 2);

        colors[2] = {r, g, b, 255};
        colors[3] = {r, g, b, 0};
    }

    for (uint32_t y = 0; y < 4; ++y) {
        uint8_t selectors = Bus::readPhysical8(address++);
        for (uint32_t x = 0; x < 4; ++x) {
            uint8_t index = (selectors >> 6) & 0x3;

            selectors <<= 2;

            const auto &c = colors[index];
            writePixel(result, originX + x, originY + y, c.r, c.g, c.b, c.a);
        }
    }
}

GXDecodedTexture GX::decodeTextureCMPR(uint32_t address, uint16_t width,
                                       uint16_t height) const {
    GXDecodedTexture result{};
    result.width = width;
    result.height = height;
    result.rgba.resize(static_cast<size_t>(width) * height * 4);

    for (uint32_t blockY = 0; blockY < height; blockY += 8) {
        for (uint32_t blockX = 0; blockX < width; blockX += 8) {
            decodeCMPRSubBlock(result, address, blockX, blockY);
            decodeCMPRSubBlock(result, address, blockX + 4, blockY);
            decodeCMPRSubBlock(result, address, blockX, blockY + 4);
            decodeCMPRSubBlock(result, address, blockX + 4, blockY + 4);
        }
    }

    return result;
}

void GX::decodeTextureMode0(uint32_t unit, uint32_t value) {
    auto &tex = state.bp.textures[unit];

    tex.wrapS = static_cast<uint8_t>(value & 0x3u);
    tex.wrapT = static_cast<uint8_t>((value >> 2) & 0x3u);

    tex.magFilter = static_cast<uint8_t>((value >> 4) & 0x1u);
    tex.minFilter = static_cast<uint8_t>((value >> 5) & 0x7u);

    int8_t rawBias = static_cast<int8_t>((value >> 9) & 0xFF);

    tex.lodBias = static_cast<float>(rawBias) / 32.0f;
}

void GX::decodeTextureImage0(uint32_t unit, uint32_t value) {
    auto &tex = state.bp.textures[unit];

    tex.width = static_cast<uint16_t>((value & 0x3FF) + 1);
    tex.height = static_cast<uint16_t>(((value >> 10) & 0x3FF) + 1);
    tex.format = static_cast<GXTextureFormat>((value >> 20) & 0xF);
}

void GX::decodeTextureImage3(uint32_t unit, uint32_t value) {
    auto &tex = state.bp.textures[unit];

    tex.address = (value & 0x00FFFFFF) << 5;
    tex.valid = true;
}

void GX::decodeTextureImage1(uint32_t unit, uint32_t value) {
    auto &tex = state.bp.textures[unit];

    tex.image1 = (value & 0x00FFFFFF) << 5;
}

void GX::decodeTextureImage2(uint32_t unit, uint32_t value) {
    auto &tex = state.bp.textures[unit];

    tex.image2 = (value & 0x00FFFFFF) << 5;
}

void GX::decodeTextureTLUT(uint32_t unit, uint32_t value) {
    auto &tlut = state.bp.textures[unit].tlut;

    tlut.address = (value & 0x3FF) << 9;
    tlut.format = static_cast<GXTLUTFormat>((value >> 10) & 0x3);
}

void GX::updateTextureUnit(uint32_t unit) {
    if (unit >= 8)
        return;

    const auto &tex = state.bp.textures[unit];
    if (!tex.valid)
        return;
    if (tex.width == 0 || tex.height == 0)
        return;

    GXDecodedTexture decoded = decodeTexture(tex);
    if (decoded.rgba.empty())
        return;

    renderer->setTexture(unit, decoded, tex);
}