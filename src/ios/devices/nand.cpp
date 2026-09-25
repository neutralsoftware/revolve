#include "ios/ios.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <array>
#include <cstdlib>
#include <limits>
#include <stdexcept>

namespace {
constexpr int32_t invalid = static_cast<int32_t>(IOSError::FS_Invalid);
constexpr int32_t missing = static_cast<int32_t>(IOSError::FS_NotFound);
constexpr int32_t ioError = static_cast<int32_t>(IOSError::FS_IO);
constexpr int32_t denied = static_cast<int32_t>(IOSError::FS_AccessDenied);

std::string guestPath(uint32_t address, uint32_t capacity = 64) {
    std::string path;
    for (uint32_t i = 0; i < std::min(capacity, 64u); ++i) {
        const char c = static_cast<char>(Bus::readPhysical8(address + i));
        if (!c)
            return path;
        path += c;
    }
    return {};
}

std::optional<std::filesystem::path> resolve(const std::string &guest) {
    if (guest.empty() || guest.front() != '/' || guest.size() >= 64)
        return std::nullopt;
    auto result = FSDevice::rootPath();
    for (const auto &component : std::filesystem::path(guest).relative_path()) {
        const auto name = component.string();
        if (name.empty() || name == "." || name == ".." || name.find('\\') != std::string::npos)
            return std::nullopt;
        result /= component;
        std::error_code ec;
        if (std::filesystem::is_symlink(std::filesystem::symlink_status(result, ec)))
            return std::nullopt;
    }
    return result;
}

int32_t fsError(const std::error_code &ec) {
    if (!ec)
        return 0;
    if (ec == std::errc::no_such_file_or_directory)
        return missing;
    if (ec == std::errc::file_exists)
        return static_cast<int32_t>(IOSError::FS_Exists);
    if (ec == std::errc::directory_not_empty)
        return static_cast<int32_t>(IOSError::FS_NotEmpty);
    if (ec == std::errc::permission_denied)
        return denied;
    return ioError;
}
}

std::filesystem::path FSDevice::rootPath() {
    static const auto root = [] {
        if (const char *configured = std::getenv("REVOLVE_NAND_PATH"))
            return std::filesystem::absolute(configured);
        char *pref = SDL_GetPrefPath("neutralsoftware", "Revolve");
        if (!pref)
            throw std::runtime_error("Cannot locate Revolve NAND directory");
        auto path = std::filesystem::path(pref) / "nand";
        SDL_free(pref);
        return path;
    }();
    return root;
}

void FSDevice::initializeNAND() {
    for (const auto *name : {"sys", "shared2/sys", "shared2/menu/FaceLib", "title", "ticket", "tmp", "import", "meta"})
        std::filesystem::create_directories(rootPath() / name);
}

int32_t FSDevice::open(const std::string &path, uint32_t mode) {
    if (path == "/dev/fs")
        return 0;
    if (mode < 1 || mode > 3)
        return invalid;
    const auto resolved = resolve(path);
    if (!resolved)
        return invalid;
    std::error_code ec;
    if (!std::filesystem::is_regular_file(*resolved, ec))
        return ec ? fsError(ec) : missing;
    auto flags = std::ios::binary | std::ios::in;
    if (mode & 2)
        flags |= std::ios::out;
    file.open(*resolved, flags);
    if (!file)
        return denied;
    filePath = *resolved;
    fileMode = mode;
    position = 0;
    return 0;
}

int32_t FSDevice::close(int32_t) {
    if (file.is_open()) {
        file.clear();
        file.close();
        if (file.fail())
            return ioError;
    }
    return 0;
}

int32_t FSDevice::read(uint32_t buffer, uint32_t size) {
    if (!file.is_open() || !(fileMode & 1))
        return denied;
    if (size > INT32_MAX || uint64_t(buffer) + size > UINT32_MAX)
        return invalid;
    file.clear();
    file.seekg(position);
    uint32_t total = 0;
    std::array<char, 4096> chunk{};
    while (total < size) {
        const auto count = std::min<uint32_t>(chunk.size(), size - total);
        file.read(chunk.data(), count);
        const auto got = static_cast<uint32_t>(file.gcount());
        for (uint32_t i = 0; i < got; ++i)
            Bus::writePhysical8(buffer + total + i, static_cast<uint8_t>(chunk[i]));
        total += got;
        if (got < count)
            break;
    }
    position += total;
    return file.bad() ? ioError : static_cast<int32_t>(total);
}

int32_t FSDevice::write(uint32_t buffer, uint32_t size) {
    if (!file.is_open() || !(fileMode & 2))
        return denied;
    if (size > INT32_MAX || uint64_t(buffer) + size > UINT32_MAX || uint64_t(position) + size > INT32_MAX)
        return invalid;
    file.clear();
    file.seekp(position);
    uint32_t total = 0;
    std::array<char, 4096> chunk{};
    while (total < size) {
        const auto count = std::min<uint32_t>(chunk.size(), size - total);
        for (uint32_t i = 0; i < count; ++i)
            chunk[i] = static_cast<char>(Bus::readPhysical8(buffer + total + i));
        file.write(chunk.data(), count);
        if (!file)
            return ioError;
        total += count;
        position += count;
    }
    file.flush();
    return file ? static_cast<int32_t>(total) : ioError;
}

int32_t FSDevice::seek(int32_t offset, uint32_t whence) {
    if (!file.is_open() || whence > 2)
        return invalid;
    std::error_code ec;
    const auto size = std::filesystem::file_size(filePath, ec);
    if (ec)
        return fsError(ec);
    int64_t next = offset;
    if (whence == 1)
        next += position;
    if (whence == 2)
        next += size;
    if (next < 0 || next > INT32_MAX || static_cast<uint64_t>(next) > size)
        return invalid;
    position = static_cast<uint32_t>(next);
    return static_cast<int32_t>(position);
}

IOSResult FSDevice::ioctl(const IOSIoctlRequest &request) {
    const auto command = static_cast<FSIOCtl>(request.request);
    std::error_code ec;
    if (command == FSIOCtl::GetFileStats) {
        if (!file.is_open() || request.outSize < 8)
            return invalid;
        const auto size = std::filesystem::file_size(filePath, ec);
        if (ec || size > UINT32_MAX)
            return ioError;
        Bus::writePhysical32(request.outPtr, static_cast<uint32_t>(size));
        Bus::writePhysical32(request.outPtr + 4, position);
        return 0;
    }
    if (file.is_open())
        return invalid;
    if (command == FSIOCtl::Shutdown)
        return 0;
    if (command == FSIOCtl::Format)
        return denied;
    const bool attributeInput = command == FSIOCtl::CreateDirectory || command == FSIOCtl::CreateFile;
    if (request.inSize < (attributeInput ? 74u : 64u))
        return invalid;
    const auto path = resolve(guestPath(request.inPtr + (attributeInput ? 6 : 0)));
    if (!path || *path == rootPath())
        return invalid;
    switch (command) {
    case FSIOCtl::CreateDirectory:
    case FSIOCtl::CreateFile:
        if (std::filesystem::exists(*path, ec))
            return static_cast<int32_t>(IOSError::FS_Exists);
        if (command == FSIOCtl::CreateDirectory) {
            std::filesystem::create_directory(*path, ec);
            return fsError(ec);
        } else {
            std::ofstream created(*path, std::ios::binary);
            return created ? 0 : ioError;
        }
    case FSIOCtl::Delete:
        if (!std::filesystem::remove(*path, ec))
            return ec ? fsError(ec) : missing;
        return 0;
    case FSIOCtl::Rename: {
        if (request.inSize < 128)
            return invalid;
        const auto destination = resolve(guestPath(request.inPtr + 64));
        if (!destination || *destination == rootPath())
            return invalid;
        std::filesystem::rename(*path, *destination, ec);
        return fsError(ec);
    }
    default:
        return invalid;
    }
}

IOSResult FSDevice::ioctlv(const IOSIoctlvRequest &request,
                           const std::vector<IOSVector> &vectors) {
    if (file.is_open() || vectors.empty() || vectors[0].size < 1)
        return invalid;
    const auto path = resolve(guestPath(vectors[0].address, vectors[0].size));
    if (!path)
        return invalid;
    std::error_code ec;
    if (!std::filesystem::exists(*path, ec))
        return ec ? fsError(ec) : missing;
    if (request.request == static_cast<uint32_t>(FSIOCtl::ReadDirectory)) {
        const bool names = request.inCount == 2 && request.outCount == 2 && vectors.size() == 4;
        const bool countOnly = request.inCount == 1 && request.outCount == 1 && vectors.size() == 2;
        if ((!names && !countOnly) || vectors.back().size < 4 || (names && vectors[1].size < 4))
            return invalid;
        std::vector<std::string> entries;
        std::filesystem::directory_iterator iterator(*path, ec);
        if (ec)
            return fsError(ec);
        for (const auto &entry : iterator) {
            if (!entry.is_symlink(ec))
                entries.push_back(entry.path().filename().string());
        }
        std::sort(entries.begin(), entries.end());
        uint32_t count = entries.size();
        if (names) {
            count = std::min(count, Bus::readPhysical32(vectors[1].address));
            uint32_t cursor = 0;
            for (uint32_t i = 0; i < count; ++i) {
                if (entries[i].size() + 1 > vectors[2].size - cursor)
                    return invalid;
                for (char c : entries[i])
                    Bus::writePhysical8(vectors[2].address + cursor++, static_cast<uint8_t>(c));
                Bus::writePhysical8(vectors[2].address + cursor++, 0);
            }
        }
        Bus::writePhysical32(vectors.back().address, count);
        return 0;
    }
    if (request.request == static_cast<uint32_t>(FSIOCtl::GetUsage)) {
        if (request.inCount != 1 || request.outCount != 2 || vectors.size() != 3 || vectors[1].size < 4 || vectors[2].size < 4)
            return invalid;
        uint64_t blocks = 0;
        uint32_t inodes = 1;
        std::filesystem::recursive_directory_iterator iterator(*path, ec);
        if (ec)
            return fsError(ec);
        for (const auto &entry : iterator) {
            if (entry.is_symlink(ec))
                continue;
            ++inodes;
            if (entry.is_regular_file(ec))
                blocks += (entry.file_size(ec) + 16383) / 16384;
            if (ec)
                return fsError(ec);
        }
        Bus::writePhysical32(vectors[1].address, static_cast<uint32_t>(blocks));
        Bus::writePhysical32(vectors[2].address, inodes);
        return 0;
    }
    return invalid;
}
