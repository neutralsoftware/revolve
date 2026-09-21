#include "ios/ios.h"
#include "core/memory.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

IOSRequest IOS::parseRequest(uint32_t address) {
    IOSRequest request{};

    request.address = address;

    request.command =
        static_cast<IOSCommand>(Bus::readPhysical32(address + 0x00));
    request.result = static_cast<int32_t>(Bus::readPhysical32(address + 0x04));
    request.fd = static_cast<int32_t>(Bus::readPhysical32(address + 0x08));
    request.args[0] = Bus::readPhysical32(address + 0x0C);
    request.args[1] = Bus::readPhysical32(address + 0x10);
    request.args[2] = Bus::readPhysical32(address + 0x14);
    request.args[3] = Bus::readPhysical32(address + 0x18);
    request.args[4] = Bus::readPhysical32(address + 0x1C);

    return request;
}

void IOS::submitRequest(uint32_t address) {
    IOSRequest request = parseRequest(address);

    Logger::log("IOS", LogLevel::Info,
                "IPC request @ 0x" + utils::toHexString(address) + " cmd=" +
                    std::to_string(static_cast<uint32_t>(request.command)) +
                    " fd=" + std::to_string(request.fd));

    int32_t result = dispatch(request);

    Bus::writePhysical32(address + 0x04, static_cast<uint32_t>(result));
}

int32_t IOS::dispatch(const IOSRequest &request) {
    switch (request.command) {
    case IOSCommand::Open: {
        IOSOpenRequest open = parseOpenRequest(request);
        std::string path = readGuestString(open.pathAddress);

        Logger::log("IOS", LogLevel::Info,
                    "OPEN path=\"" + path +
                        "\" mode=" + std::to_string(open.mode));

        int32_t fd = allocateFileDescriptor(path);
        return fd;
    }

    case IOSCommand::Close: {
        Logger::log("IOS", LogLevel::Info,
                    "CLOSE fd=" + std::to_string(request.fd));
        auto it = fileDescriptors.find(request.fd);
        if (it == fileDescriptors.end()) {
            return -1;
        }
        fileDescriptors.erase(it);

        return 0;
    }

    case IOSCommand::Read: {
        IOSReadRequest read = parseReadRequest(request);

        Logger::log("IOS", LogLevel::Info,
                    "READ fd=" + std::to_string(request.fd) + " buffer=0x" +
                        utils::toHexString(read.bufferAddress) +
                        " size=" + std::to_string(read.size));

        return -1;
    }

    case IOSCommand::Write: {
        IOSWriteRequest write = parseWriteRequest(request);

        Logger::log("IOS", LogLevel::Info,
                    "WRITE fd=" + std::to_string(request.fd) + " buffer=0x" +
                        utils::toHexString(write.bufferAddress) +
                        " size=" + std::to_string(write.size));

        return -1;
    }

    case IOSCommand::Seek: {
        IOSSeekRequest seek = parseSeekRequest(request);

        Logger::log("IOS", LogLevel::Info,
                    "SEEK fd=" + std::to_string(request.fd) +
                        " offset=" + std::to_string(seek.offset) +
                        " whence=" + std::to_string(seek.whence));

        return -1;
    }

    case IOSCommand::Ioctl: {
        IOSIoctlRequest ioctl = parseIoctlRequest(request);

        Logger::log("IOS", LogLevel::Info,
                    "IOCTL fd=" + std::to_string(request.fd) +
                        " request=" + std::to_string(ioctl.request));

        return -1;
    }

    case IOSCommand::Ioctlv: {
        IOSIoctlvRequest ioctlv = parseIoctlvRequest(request);

        auto vectors = parseVectors(ioctlv.vectorsAddress,
                                    ioctlv.inCount + ioctlv.outCount);

        Logger::log("IOS", LogLevel::Info,
                    "IOCTLV fd=" + std::to_string(request.fd) +
                        " request=" + std::to_string(ioctlv.request) +
                        " in=" + std::to_string(ioctlv.inCount) +
                        " out=" + std::to_string(ioctlv.outCount));

        return -1;
    }

    default:
        Logger::log("IOS", LogLevel::Warning,
                    "Unknown IOS command " +
                        std::to_string(static_cast<uint32_t>(request.command)));

        return -1;
    }
}

IOSOpenRequest IOS::parseOpenRequest(const IOSRequest &request) {
    IOSOpenRequest openRequest{};

    openRequest.pathAddress = request.args[0];
    openRequest.mode = request.args[1];

    return openRequest;
}

IOSReadRequest IOS::parseReadRequest(const IOSRequest &request) {
    IOSReadRequest readRequest{};

    readRequest.bufferAddress = request.args[0];
    readRequest.size = request.args[1];

    return readRequest;
}

IOSWriteRequest IOS::parseWriteRequest(const IOSRequest &request) {
    IOSWriteRequest writeRequest{};

    writeRequest.bufferAddress = request.args[0];
    writeRequest.size = request.args[1];

    return writeRequest;
}

IOSSeekRequest IOS::parseSeekRequest(const IOSRequest &request) {
    IOSSeekRequest seekRequest{};

    seekRequest.offset = request.args[0];
    seekRequest.whence = request.args[1];

    return seekRequest;
}

IOSIoctlRequest IOS::parseIoctlRequest(const IOSRequest &request) {
    IOSIoctlRequest ioctlRequest{};

    ioctlRequest.request = request.args[0];
    ioctlRequest.inPtr = request.args[1];
    ioctlRequest.inSize = request.args[2];
    ioctlRequest.outPtr = request.args[3];
    ioctlRequest.outSize = request.args[4];

    return ioctlRequest;
}

IOSIoctlvRequest IOS::parseIoctlvRequest(const IOSRequest &request) {
    IOSIoctlvRequest ioctlvRequest{};

    ioctlvRequest.request = request.args[0];
    ioctlvRequest.inCount = request.args[1];
    ioctlvRequest.outCount = request.args[2];
    ioctlvRequest.vectorsAddress = request.args[3];

    return ioctlvRequest;
}

std::vector<IOSVector> IOS::parseVectors(uint32_t address, uint32_t count) {
    std::vector<IOSVector> vectors;
    vectors.reserve(count);

    for (uint32_t i = 0; i < count; ++i) {
        uint32_t vectorAddress = address + i * sizeof(IOSVector);
        IOSVector vector{};
        vector.address = Bus::readPhysical32(vectorAddress);
        vector.size = Bus::readPhysical32(vectorAddress + 4);
        vectors.push_back(vector);
    }

    return vectors;
}

std::string IOS::readGuestString(uint32_t address, size_t maxLength) {
    std::string result;

    for (size_t i = 0; i < maxLength; ++i) {
        char c = static_cast<char>(Bus::readPhysical8(address + i));

        if (c == '\0')
            return result;

        result += c;
    }

    throw std::runtime_error("Unterminated IOS guest string");
}

int32_t IOS::allocateFileDescriptor(const std::string &path) {
    int32_t fd = nextFileDescriptor++;
    fileDescriptors.emplace(fd, IOSFileDescriptor{.path = path});
    return fd;
}