#include "ios/ios.h"
#include "device.h"
#include <array>
#include <cstdio>
#include <fstream>

namespace {
constexpr int32_t invalid = static_cast<int32_t>(IOSError::ES_Invalid);
std::string titleDirectory(uint64_t title) {
    std::array<char, 40> path{};
    std::snprintf(path.data(), path.size(), "/title/%08x/%08x/data",
                  static_cast<uint32_t>(title >> 32), static_cast<uint32_t>(title));
    return path.data();
}
uint64_t readTitle(uint32_t address) {
    return (uint64_t(Bus::readPhysical32(address)) << 32) | Bus::readPhysical32(address + 4);
}
}

void ESDevice::prepareDiscBoot(uint64_t partitionOffset) {
    std::array<uint8_t, 8> title{};
    if (!Device::globalDevice->disc->readRaw(partitionOffset + 0x1DC, title))
        return;
    currentTitle = 0;
    for (auto byte : title)
        currentTitle = (currentTitle << 8) | byte;
    std::filesystem::create_directories(FSDevice::rootPath() / titleDirectory(currentTitle).substr(1));
}

IOSResult ESDevice::ioctlv(const IOSIoctlvRequest &request,
                           const std::vector<IOSVector> &vectors) {
    const auto shape = [&](uint32_t in, uint32_t out) {
        return request.inCount == in && request.outCount == out && vectors.size() == in + out;
    };
    switch (request.request) {
    case 0x20:
        if (!shape(0, 1) || vectors[0].size != 8)
            return invalid;
        Bus::writePhysical32(vectors[0].address, currentTitle >> 32);
        Bus::writePhysical32(vectors[0].address + 4, currentTitle);
        return 0;
    case 0x1D: {
        if (!shape(1, 1) || vectors[0].size != 8 || vectors[1].size < 30)
            return invalid;
        const auto path = titleDirectory(readTitle(vectors[0].address));
        for (size_t i = 0; i <= path.size(); ++i)
            Bus::writePhysical8(vectors[1].address + i, i == path.size() ? 0 : path[i]);
        return 0;
    }
    case 0x21:
        if (!shape(1, 0) || vectors[0].size != 8)
            return invalid;
        if (readTitle(vectors[0].address) != currentTitle)
            return static_cast<int32_t>(IOSError::ES_AccessDenied);
        return 0;
    case 0x34:
    case 0x35: {
        if (!shape(1, 1) || vectors[0].size != 8)
            return invalid;
        const auto title = readTitle(vectors[0].address);
        const auto path = FSDevice::rootPath() / titleDirectory(title).substr(1) / "../content/title.tmd";
        std::error_code ec;
        const auto length = std::filesystem::file_size(path, ec);
        if (ec)
            return static_cast<int32_t>(IOSError::FS_NotFound);
        if (length > 4 * 1024 * 1024)
            return invalid;
        if (request.request == 0x34) {
            if (vectors[1].size != 4)
                return invalid;
            Bus::writePhysical32(vectors[1].address, static_cast<uint32_t>(length));
            return 0;
        }
        if (vectors[1].size < length)
            return invalid;
        std::ifstream file(path, std::ios::binary);
        for (uint32_t i = 0; i < length; ++i) {
            char value;
            if (!file.get(value))
                return static_cast<int32_t>(IOSError::ES_IO);
            Bus::writePhysical8(vectors[1].address + i, static_cast<uint8_t>(value));
        }
        return 0;
    }
    default:
        return invalid;
    }
}

int32_t ESDevice::open(const std::string &, uint32_t) { return 0; }
int32_t ESDevice::close(int32_t) { return 0; }
