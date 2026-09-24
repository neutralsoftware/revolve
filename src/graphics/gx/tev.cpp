
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

void GX::decodeTevOrder(uint8_t reg, uint32_t value) {
    const uint32_t pair = reg - 0x28;

    const uint32_t stage0 = pair * 2;
    const uint32_t stage1 = stage0 + 1;

    decodeTevOrderStage(stage0, value & 0xFFF);
    decodeTevOrderStage(stage1, (value >> 12) & 0xFFF);
}

void GX::decodeTevOrderStage(uint32_t stage, uint32_t raw) {
    if (stage >= 16)
        return;

    auto &order = state.bp.tevStages[stage].order;
    order.texMap = static_cast<uint8_t>(raw & 0x7);
    order.texCoord = static_cast<uint8_t>((raw >> 3) & 0x7);
    order.textureEnabled = (raw & (1u << 6)) != 0;
    order.colorChannel = static_cast<uint8_t>((raw >> 7) & 0x7);
}

void GX::decodeTevColorCombiner(uint32_t stage, uint32_t value) {
    auto &c = state.bp.tevStages[stage].color;

    c.d = static_cast<GXTevColorArg>(value & 0xFu);
    c.c = static_cast<GXTevColorArg>((value >> 4) & 0xFu);
    c.b = static_cast<GXTevColorArg>((value >> 8) & 0xFu);
    c.a = static_cast<GXTevColorArg>((value >> 12) & 0xFu);
    c.bias = static_cast<GXTevBias>((value >> 16) & 0x3u);
    c.op = static_cast<GXTevOp>((value >> 18) & 0x1u);
    c.clamp = ((value >> 19) & 1u) != 0;
    c.scale = static_cast<GXTevScale>((value >> 20) & 0x3u);
    c.output = static_cast<GXTevOutput>((value >> 22) & 0x3u);
}

void GX::decodeTevAlphaCombiner(uint32_t stage, uint32_t value) {
    auto &a = state.bp.tevStages[stage].alpha;

    a.rasterSwap = static_cast<uint8_t>(value & 0x3u);
    a.textureSwap = static_cast<uint8_t>((value >> 2) & 0x3u);
    a.d = static_cast<GXTevAlphaArg>((value >> 4) & 0x7u);
    a.c = static_cast<GXTevAlphaArg>((value >> 7) & 0x7u);
    a.b = static_cast<GXTevAlphaArg>((value >> 10) & 0x7u);
    a.a = static_cast<GXTevAlphaArg>((value >> 13) & 0x7u);
    a.bias = static_cast<GXTevBias>((value >> 16) & 0x3u);
    a.op = static_cast<GXTevOp>((value >> 18) & 0x1u);
    a.clamp = ((value >> 19) & 1u) != 0;
    a.scale = static_cast<GXTevScale>((value >> 20) & 0x3u);
    a.output = static_cast<GXTevOutput>((value >> 22) & 0x3u);
}

GXColor GX::calculateLight(const GXLight &light, uint32_t control,
                           const GXVec3 &position, const GXVec3 &normal) const {
    const uint32_t diffuseFunction = (control >> 7) & 0x3;
    const uint32_t attenuationFunction = (control >> 9) & 0x3;

    GXVec3 lightDirection{};
    float attenuation = 1.0f;

    if (attenuationFunction == 0 || attenuationFunction == 2) {
        lightDirection = gx::normalize(light.position - position);

        attenuation = 1.0f;

        if (gx::length(lightDirection) == 0.0f)
            lightDirection = normal;
    } else if (attenuationFunction == 1) {
        lightDirection = gx::normalize(light.position - position);

        float d = gx::dot(normal, lightDirection);
        float x = 0.0f;

        if (d >= 0.0f) {
            x = std::max(0.0f, gx::dot(normal, light.direction));
        }

        GXVec3 distAtt = light.distAttenuation;
        if (diffuseFunction != 0) {
            distAtt = gx::normalize(distAtt);
        }

        const float numerator =
            std::max(0.0f, light.cosAttenuation.x + light.cosAttenuation.y * x +
                               light.cosAttenuation.z * x * x);
        const float denominator = distAtt.x + distAtt.y * x + distAtt.z * x * x;

        attenuation = denominator != 0.0f ? numerator / denominator : 0.0f;
    } else if (attenuationFunction == 3) {
        GXVec3 delta = light.position - position;

        const float distanceSquared = gx::dot(delta, delta);

        const float distance = std::sqrt(distanceSquared);

        if (distance > 0.0f) {
            lightDirection = {delta.x / distance, delta.y / distance,
                              delta.z / distance};
        } else {
            lightDirection = normal;
        }

        const float angle =
            std::max(0.0f, gx::dot(lightDirection, light.direction));

        const float numerator = std::max(
            0.0f, light.cosAttenuation.x + light.cosAttenuation.y * angle +
                      light.cosAttenuation.z * angle * angle);

        const float denominator = light.distAttenuation.x +
                                  light.distAttenuation.y * distance +
                                  light.distAttenuation.z * distanceSquared;

        attenuation = denominator != 0.0f ? numerator / denominator : 0.0f;
    }

    float diffuse = 1.0f;

    switch (diffuseFunction) {
    case 0:
        // GX_DF_NONE
        diffuse = 1.0f;
        break;

    case 1:
        // GX_DF_SIGN
        diffuse = gx::dot(lightDirection, normal);
        break;

    case 2:
        // GX_DF_CLAMP
        diffuse = std::max(0.0f, gx::dot(lightDirection, normal));
        break;

    default:
        diffuse = 0.0f;
        break;
    }

    const float scale = attenuation * diffuse;

    return {light.color.r * scale, light.color.g * scale, light.color.b * scale,
            light.color.a * scale};
}

GXColor GX::getVertexColor(const GXVertex &vertex, uint32_t channel) const {
    const auto &vcd = state.cp.getVCD();

    if (channel == 0) {
        if (vcd.color0 != GXVertexAttributeMode::None)
            return vertex.color0;

        if (vcd.color1 != GXVertexAttributeMode::None)
            return vertex.color1;
    } else {
        if (vcd.color1 != GXVertexAttributeMode::None)
            return vertex.color1;

        if (vcd.color0 != GXVertexAttributeMode::None)
            return vertex.color0;
    }

    return {1, 1, 1, 1};
}

GXColor GX::calculateLightingChannel(const GXVertex &vertex, uint32_t channel,
                                     const GXVec3 &viewPosition,
                                     const GXVec3 &normal) const {
    if (channel >= 2)
        return {};

    const auto &lighting = state.xf.lightingChannels[channel];

    const uint32_t colorControl = lighting.colorControl;
    const uint32_t alphaControl = lighting.alphaControl;

    const GXColor vertexColor = getVertexColor(vertex, channel);

    GXColor material = state.xf.materialColors[channel];

    if (colorControl & 1u) {
        material.r = vertexColor.r;
        material.g = vertexColor.g;
        material.b = vertexColor.b;
    }

    if (alphaControl & 1u) {
        material.a = vertexColor.a;
    }

    GXColor accumulated{1.0f, 1.0f, 1.0f, 1.0f};
    if (colorControl & (1u << 1)) {
        GXColor ambient = state.xf.ambientColors[channel];
        if (colorControl & (1u << 6)) {
            ambient.r = vertexColor.r;
            ambient.g = vertexColor.g;
            ambient.b = vertexColor.b;
        }

        accumulated.r = ambient.r;
        accumulated.g = ambient.g;
        accumulated.b = ambient.b;

        const uint8_t mask = gx::getGXLightMask(colorControl);

        for (uint32_t i = 0; i < 8; ++i) {
            if ((mask & (1u << i)) == 0)
                continue;

            GXColor contribution = calculateLight(
                state.xf.lights[i], colorControl, viewPosition, normal);

            accumulated.r += contribution.r;
            accumulated.g += contribution.g;
            accumulated.b += contribution.b;
        }
    }

    if (alphaControl & (1u << 1)) {
        float ambientAlpha = state.xf.ambientColors[channel].a;

        if (alphaControl & (1u << 6))
            ambientAlpha = vertexColor.a;

        accumulated.a = ambientAlpha;

        const uint8_t mask = gx::getGXLightMask(alphaControl);

        for (uint32_t i = 0; i < 8; ++i) {
            if ((mask & (1u << i)) == 0)
                continue;

            const GXColor contribution = calculateLight(
                state.xf.lights[i], alphaControl, viewPosition, normal);

            accumulated.a += contribution.a;
        }
    }

    accumulated.r = std::clamp(accumulated.r, 0.0f, 1.0f);
    accumulated.g = std::clamp(accumulated.g, 0.0f, 1.0f);
    accumulated.b = std::clamp(accumulated.b, 0.0f, 1.0f);
    accumulated.a = std::clamp(accumulated.a, 0.0f, 1.0f);

    GXColor result{};

    result.r = material.r * accumulated.r;
    result.g = material.g * accumulated.g;
    result.b = material.b * accumulated.b;
    result.a = material.a * accumulated.a;

    return result;
}

void GX::decodeTevRegister(uint8_t reg, uint32_t value) {
    if (reg < 0xE0 || reg > 0xE7)
        return;

    const uint32_t index = (reg - 0xE0) / 2;
    const bool high = (reg & 1) != 0;
    const bool konst = (value & (1u << 23)) != 0;

    auto &target =
        konst ? state.bp.konstRegisters[index] : state.bp.tevRegisters[index];

    if (!high) {
        target.r = gx::signExtend11(value >> 0);
        target.a = gx::signExtend11(value >> 12);
    } else {
        target.b = gx::signExtend11(value >> 0);
        target.g = gx::signExtend11(value >> 12);
    }
}

void GX::decodeTevKSel(uint8_t reg, uint32_t value) {
    if (reg < 0xF6 || reg > 0xFD)
        return;

    const uint32_t pair = reg - 0xF6;
    const uint32_t stage0 = pair * 2;
    const uint32_t stage1 = stage0 + 1;

    if (stage0 < 16) {
        state.bp.tevStages[stage0].konstColorSel =
            static_cast<uint8_t>((value >> 4) & 0x1F);

        state.bp.tevStages[stage0].konstAlphaSel =
            static_cast<uint8_t>((value >> 9) & 0x1F);
    }

    if (stage1 < 16) {
        state.bp.tevStages[stage1].konstColorSel =
            static_cast<uint8_t>((value >> 14) & 0x1F);

        state.bp.tevStages[stage1].konstAlphaSel =
            static_cast<uint8_t>((value >> 19) & 0x1F);
    }
}