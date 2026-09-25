#include "device.h"
#include "ios/ios.h"
#include "ios/title.h"
#include <algorithm>
#include <array>
#include <charconv>
#include <openssl/sha.h>

namespace {
constexpr int32_t invalid = static_cast<int32_t>(IOSError::ES_Invalid);
uint64_t readTitle(uint32_t address) {
    return (uint64_t(Bus::readPhysical32(address)) << 32) |
           Bus::readPhysical32(address + 4);
}
void writeTitle(uint32_t address, uint64_t title) {
    Bus::writePhysical32(address, title >> 32);
    Bus::writePhysical32(address + 4, title);
}
void writeBytes(uint32_t address, std::span<const uint8_t> data) {
    for (size_t i = 0; i < data.size(); ++i)
        Bus::writePhysical8(address + i, data[i]);
}
std::optional<uint32_t> parseHex(const std::string &name) {
    if (name.size() != 8)
        return std::nullopt;
    uint32_t value = 0;
    const auto result =
        std::from_chars(name.data(), name.data() + name.size(), value, 16);
    if (result.ec != std::errc{} || result.ptr != name.data() + name.size())
        return std::nullopt;
    return value;
}
std::vector<uint64_t> installedTitles(bool owned) {
    std::vector<uint64_t> titles;
    std::error_code error;
    std::filesystem::directory_iterator groups(FSDevice::rootPath() / "title",
                                               error);
    if (error)
        return titles;
    for (const auto &group : groups) {
        const auto high = parseHex(group.path().filename().string());
        if (!high || !group.is_directory(error) || group.is_symlink(error))
            continue;
        std::filesystem::directory_iterator entries(group.path(), error);
        if (error)
            continue;
        for (const auto &entry : entries) {
            const auto low = parseHex(entry.path().filename().string());
            if (!low || !entry.is_directory(error) || entry.is_symlink(error))
                continue;
            const uint64_t title = (uint64_t(*high) << 32) | *low;
            if (!std::filesystem::is_regular_file(
                    entry.path() / "content/title.tmd", error))
                continue;
            if (owned &&
                !std::filesystem::is_regular_file(nandTicketPath(title), error))
                continue;
            titles.push_back(title);
        }
    }
    std::sort(titles.begin(), titles.end());
    return titles;
}
std::vector<uint8_t> ticketView(std::span<const uint8_t> ticket) {
    if (ticket.size() != 0x2A4 || ticket[0] != 0 || ticket[1] != 1 ||
        ticket[2] != 0 || ticket[3] != 1)
        return {};
    std::vector<uint8_t> view(0xD8);
    std::copy_n(ticket.begin() + 0x1D0, 0xD4, view.begin() + 4);
    return view;
}
}

std::vector<uint8_t> ESDevice::titleMetadata(uint64_t title) const {
    if (title == discTitle && !discTmd.empty())
        return discTmd;
    return readTitleFile(nandTitlePath(title) / "content/title.tmd",
                         4 * 1024 * 1024);
}

std::vector<uint8_t> ESDevice::titleTicket(uint64_t title) const {
    if (title == discTitle && !discTicket.empty())
        return discTicket;
    return readTitleFile(nandTicketPath(title), 0x2A4);
}

void ESDevice::prepareDiscBoot(uint64_t partitionOffset) {
    auto &disc = *Device::globalDevice->disc;
    const uint32_t length = disc.readBE32(partitionOffset + 0x2A4);
    const uint64_t offset = uint64_t(disc.readBE32(partitionOffset + 0x2A8))
                            << 2;
    if (length > 4 * 1024 * 1024)
        return;
    std::vector<uint8_t> tmd(length);
    std::vector<uint8_t> ticket(0x2A4);
    if (!disc.readRaw(partitionOffset + offset, tmd) ||
        !disc.readRaw(partitionOffset, ticket))
        return;
    const auto metadata = TitleMetadata::parse(tmd);
    if (!metadata)
        return;
    currentTitle = discTitle = metadata->id;
    discTmd = std::move(tmd);
    discTicket = std::move(ticket);
    std::filesystem::create_directories(nandTitlePath(currentTitle) / "data");
}

std::optional<Executable> ESDevice::loadTitleExecutable(uint64_t title) {
    const auto tmd = titleMetadata(title);
    const auto metadata = TitleMetadata::parse(tmd);
    if (!metadata || metadata->id != title)
        return std::nullopt;
    const auto boot =
        std::find_if(metadata->contents.begin(), metadata->contents.end(),
                     [&](const auto &content) {
                         return content.index == metadata->bootIndex;
                     });
    if (boot == metadata->contents.end() || boot->size > 64 * 1024 * 1024)
        return std::nullopt;
    const auto path = nandContentPath(title, *boot);
    if (!path)
        return std::nullopt;
    auto bytes = readTitleFile(*path, boot->size);
    if (bytes.size() != boot->size)
        return std::nullopt;
    std::array<uint8_t, SHA_DIGEST_LENGTH> hash{};
    SHA1(bytes.data(), bytes.size(), hash.data());
    if (hash != boot->hash)
        return std::nullopt;
    try {
        auto executable = Executable::parseFromDolphin(std::move(bytes));
        currentTitle = title;
        contentFiles.clear();
        std::filesystem::create_directories(nandTitlePath(title) / "data");
        Bus::writePhysical32(0x3140, static_cast<uint32_t>(metadata->ios));
        return executable;
    } catch (const std::exception &) {
        return std::nullopt;
    }
}

int32_t ESDevice::openContent(uint64_t title, uint32_t index) {
    const auto tmd = titleMetadata(title);
    const auto metadata = TitleMetadata::parse(tmd);
    if (!metadata || metadata->id != title || index > UINT16_MAX)
        return invalid;
    const auto content =
        std::find_if(metadata->contents.begin(), metadata->contents.end(),
                     [&](const auto &entry) { return entry.index == index; });
    if (content == metadata->contents.end())
        return IOS::error(IOSError::FS_NotFound);
    const auto path = nandContentPath(title, *content);
    if (!path)
        return IOS::error(IOSError::FS_NotFound);
    if (contentFiles.size() >= 16 || nextContent == INT32_MAX)
        return IOS::error(IOSError::ES_FDExhausted);
    auto file = std::make_shared<FSDevice>();
    const auto result = file->open(
        "/" + path->lexically_relative(FSDevice::rootPath()).generic_string(),
        1);
    if (result < 0)
        return result;
    const int32_t descriptor = nextContent++;
    contentFiles.emplace(descriptor, std::move(file));
    return descriptor;
}

IOSResult ESDevice::ioctlv(const IOSIoctlvRequest &request,
                           const std::vector<IOSVector> &vectors) {
    const auto shape = [&](uint32_t in, uint32_t out) {
        return request.inCount == in && request.outCount == out &&
               vectors.size() == in + out;
    };
    switch (request.request) {
    case 7:
        if (!shape(0, 1) || vectors[0].size != 4)
            return invalid;
        Bus::writePhysical32(vectors[0].address, 0x04000001);
        return 0;
    case 9:
        if (!shape(1, 0) || vectors[0].size != 4)
            return invalid;
        return openContent(currentTitle,
                           Bus::readPhysical32(vectors[0].address));
    case 0x24:
        if (!shape(3, 0) || vectors[0].size != 8 || vectors[1].size != 0xD8 ||
            vectors[2].size != 4)
            return invalid;
        return openContent(readTitle(vectors[0].address),
                           Bus::readPhysical32(vectors[2].address));
    case 0x0A:
    case 0x0B:
    case 0x23: {
        if ((request.request == 0x0A && !shape(1, 1)) ||
            (request.request == 0x0B && !shape(1, 0)) ||
            (request.request == 0x23 && !shape(3, 0)) || vectors[0].size != 4)
            return invalid;
        const int32_t descriptor = Bus::readPhysical32(vectors[0].address);
        const auto it = contentFiles.find(descriptor);
        if (it == contentFiles.end())
            return invalid;
        if (request.request == 0x0A)
            return it->second->read(vectors[1].address, vectors[1].size);
        if (request.request == 0x0B) {
            contentFiles.erase(it);
            return 0;
        }
        if (vectors[1].size != 4 || vectors[2].size != 4)
            return invalid;
        return it->second->seek(
            static_cast<int32_t>(Bus::readPhysical32(vectors[1].address)),
            Bus::readPhysical32(vectors[2].address));
    }
    case 0x0C:
    case 0x0E:
    case 0x0D:
    case 0x0F: {
        const bool countOnly =
            request.request == 0x0C || request.request == 0x0E;
        if ((countOnly && (!shape(0, 1) || vectors[0].size != 4)) ||
            (!countOnly && (!shape(1, 1) || vectors[0].size != 4)))
            return invalid;
        const auto titles =
            installedTitles(request.request == 0x0C || request.request == 0x0D);
        if (countOnly) {
            Bus::writePhysical32(vectors[0].address, titles.size());
        } else {
            const uint32_t count = Bus::readPhysical32(vectors[0].address);
            if (count > vectors[1].size / 8)
                return invalid;
            for (size_t i = 0; i < std::min<size_t>(count, titles.size()); ++i)
                writeTitle(vectors[1].address + i * 8, titles[i]);
        }
        return 0;
    }
    case 0x12:
    case 0x13: {
        const bool countOnly = request.request == 0x12;
        if (!shape(countOnly ? 1 : 2, 1) || vectors[0].size != 8 ||
            vectors[1].size != 4)
            return invalid;
        const auto ticket = titleTicket(readTitle(vectors[0].address));
        const auto view = ticketView(ticket);
        if (countOnly) {
            Bus::writePhysical32(vectors[1].address, view.empty() ? 0 : 1);
            return 0;
        }
        const auto count = Bus::readPhysical32(vectors[1].address);
        if (count != 1 || vectors[2].size < view.size())
            return invalid;
        if (view.empty())
            return IOS::error(IOSError::ES_NoTicket);
        writeBytes(vectors[2].address, view);
        return 0;
    }
    case 0x14:
    case 0x15: {
        const bool countOnly = request.request == 0x14;
        if (!shape(countOnly ? 1 : 2, 1) || vectors[0].size != 8)
            return invalid;
        const auto tmd = titleMetadata(readTitle(vectors[0].address));
        const auto metadata = TitleMetadata::parse(tmd);
        if (!metadata)
            return IOS::error(IOSError::FS_NotFound);
        const uint32_t size = 0x5C + metadata->contents.size() * 16;
        if (countOnly) {
            if (vectors[1].size != 4)
                return invalid;
            Bus::writePhysical32(vectors[1].address, size);
            return 0;
        }
        if (vectors[1].size != 4 ||
            Bus::readPhysical32(vectors[1].address) < size ||
            vectors[2].size < size)
            return invalid;
        std::vector<uint8_t> view(size);
        const auto payload = metadata->payloadOffset;
        view[0] = tmd[payload + 0x40];
        std::copy_n(tmd.begin() + payload + 0x44, 0x54, view.begin() + 4);
        std::copy_n(tmd.begin() + payload + 0x9C, 4, view.begin() + 0x58);
        for (size_t i = 0; i < metadata->contents.size(); ++i)
            std::copy_n(tmd.begin() + payload + 0xA4 + i * 36, 16,
                        view.begin() + 0x5C + i * 16);
        writeBytes(vectors[2].address, view);
        return 0;
    }
    case 0x20:
        if (!shape(0, 1) || vectors[0].size != 8)
            return invalid;
        writeTitle(vectors[0].address, currentTitle);
        return 0;
    case 0x1D: {
        if (!shape(1, 1) || vectors[0].size != 8 || vectors[1].size < 30)
            return invalid;
        const auto path =
            "/" + (nandTitlePath(readTitle(vectors[0].address)) / "data")
                      .lexically_relative(FSDevice::rootPath())
                      .generic_string();
        for (size_t i = 0; i <= path.size(); ++i)
            Bus::writePhysical8(vectors[1].address + i,
                                i == path.size() ? 0 : path[i]);
        return 0;
    }
    case 0x21:
        if (!shape(1, 0) || vectors[0].size != 8)
            return invalid;
        if (readTitle(vectors[0].address) != currentTitle)
            return IOS::error(IOSError::ES_AccessDenied);
        return 0;
    case 0x34:
    case 0x35: {
        const bool countOnly = request.request == 0x34;
        if (!shape(countOnly ? 1 : 2, 1) || vectors[0].size != 8)
            return invalid;
        const auto tmd = titleMetadata(readTitle(vectors[0].address));
        if (tmd.empty())
            return IOS::error(IOSError::FS_NotFound);
        if (countOnly) {
            if (vectors[1].size != 4)
                return invalid;
            Bus::writePhysical32(vectors[1].address, tmd.size());
            return 0;
        }
        if (vectors[1].size != 4 ||
            Bus::readPhysical32(vectors[1].address) < tmd.size() ||
            vectors[2].size < tmd.size())
            return invalid;
        writeBytes(vectors[2].address, tmd);
        return 0;
    }
    default:
        return invalid;
    }
}

int32_t ESDevice::open(const std::string &, uint32_t) { return 0; }
int32_t ESDevice::close(int32_t) {
    contentFiles.clear();
    return 0;
}
