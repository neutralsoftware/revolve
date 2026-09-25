#include "ios/title.h"
#include "ios/ios.h"
#include <algorithm>
#include <cstdio>
#include <fstream>

namespace {
uint16_t read16(std::span<const uint8_t> data, size_t offset) {
    return (uint16_t(data[offset]) << 8) | data[offset + 1];
}
uint32_t read32(std::span<const uint8_t> data, size_t offset) {
    return (uint32_t(read16(data, offset)) << 16) | read16(data, offset + 2);
}
uint64_t read64(std::span<const uint8_t> data, size_t offset) {
    return (uint64_t(read32(data, offset)) << 32) | read32(data, offset + 4);
}
std::string hex32(uint32_t value) {
    std::array<char, 9> text{};
    std::snprintf(text.data(), text.size(), "%08x", value);
    return text.data();
}
}

std::optional<TitleMetadata>
TitleMetadata::parse(std::span<const uint8_t> data) {
    if (data.size() < 4)
        return std::nullopt;
    uint32_t payload = 0;
    switch (read32(data, 0)) {
    case 0x00010000:
        payload = 0x240;
        break;
    case 0x00010001:
        payload = 0x140;
        break;
    case 0x00010002:
        payload = 0x80;
        break;
    default:
        return std::nullopt;
    }
    if (data.size() < payload + 0xA4)
        return std::nullopt;
    const uint16_t count = read16(data, payload + 0x9E);
    if (count > (data.size() - payload - 0xA4) / 36)
        return std::nullopt;
    TitleMetadata result;
    result.id = read64(data, payload + 0x4C);
    result.ios = read64(data, payload + 0x44);
    result.bootIndex = read16(data, payload + 0xA0);
    result.payloadOffset = payload;
    result.contents.reserve(count);
    for (uint16_t i = 0; i < count; ++i) {
        const size_t at = payload + 0xA4 + i * 36;
        TitleContent content;
        content.id = read32(data, at);
        content.index = read16(data, at + 4);
        content.type = read16(data, at + 6);
        content.size = read64(data, at + 8);
        std::copy_n(data.begin() + at + 16, 20, content.hash.begin());
        if (std::any_of(result.contents.begin(), result.contents.end(),
                        [&](const auto &other) {
                            return other.index == content.index ||
                                   other.id == content.id;
                        }))
            return std::nullopt;
        result.contents.push_back(content);
    }
    return result;
}

std::filesystem::path nandTitlePath(uint64_t title) {
    return FSDevice::rootPath() / "title" / hex32(title >> 32) / hex32(title);
}

std::filesystem::path nandTicketPath(uint64_t title) {
    return FSDevice::rootPath() / "ticket" / hex32(title >> 32) /
           (hex32(title) + ".tik");
}

std::optional<std::filesystem::path>
nandContentPath(uint64_t title, const TitleContent &content) {
    if (!(content.type & 0x8000))
        return nandTitlePath(title) / "content" / (hex32(content.id) + ".app");
    const auto map =
        readTitleFile(FSDevice::rootPath() / "shared1/content.map", 28 * 65536);
    for (size_t offset = 0; offset + 28 <= map.size(); offset += 28) {
        if (!std::equal(content.hash.begin(), content.hash.end(),
                        map.begin() + offset + 8))
            continue;
        std::string name(map.begin() + offset, map.begin() + offset + 8);
        if (name.find_first_not_of("0123456789abcdefABCDEF") !=
            std::string::npos)
            return std::nullopt;
        return FSDevice::rootPath() / "shared1" / (name + ".app");
    }
    return std::nullopt;
}

std::vector<uint8_t> readTitleFile(const std::filesystem::path &path,
                                   uint64_t maximum) {
    std::error_code error;
    const uint64_t size = std::filesystem::file_size(path, error);
    if (error || size > maximum)
        return {};
    std::ifstream file(path, std::ios::binary);
    std::vector<uint8_t> result(size);
    file.read(reinterpret_cast<char *>(result.data()), result.size());
    return file ? result : std::vector<uint8_t>{};
}
