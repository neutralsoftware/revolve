#include "ios/ios.h"
#include "core/memory.h"
#include "device.h"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

void IOS::init() {
    FSDevice::initializeNAND();
    registerDevice("/dev/stm/immediate",
                   std::make_shared<STMImmediateDevice>());
    registerDevice("/dev/stm/eventhook",
                   std::make_shared<STMEventHookDevice>());
    registerDevice("/dev/es", std::make_shared<ESDevice>());
    registerDevice("/dev/fs", std::make_shared<FSDevice>());
    registerDevice("/dev/di",
                   std::make_shared<DIDevice>(Device::globalDevice->memory,
                                              Device::globalDevice->disc));
    bluetoothDevice = std::make_shared<BluetoothUSBDevice>(
        *this, *Device::globalDevice->inputManager);
    registerDevice("/dev/usb/oh1/57e/305", bluetoothDevice);
}

void IOS::update() {
    if (bluetoothDevice)
        bluetoothDevice->update();
}

void IOS::registerDevice(const std::string &path,
                         std::shared_ptr<IOSDevice> device) {
    devices[path] = device;
}

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

    IOSResult result = dispatch(request);

    if (!result.has_value())
        return;

    completeRequest(address, *result);
}

IOSResult IOS::dispatch(const IOSRequest &request) {
    switch (request.command) {
    case IOSCommand::Open: {
        IOSOpenRequest open = parseOpenRequest(request);
        std::string path = readGuestString(open.pathAddress);

        Logger::log("IOS", LogLevel::Info,
                    "OPEN path=\"" + path +
                        "\" mode=" + std::to_string(open.mode));

        auto it = devices.find(path);

        std::shared_ptr<IOSDevice> device;
        if (it != devices.end()) {
            device = it->second;
        } else if (path.starts_with("/") && !path.starts_with("/dev/")) {
            device = std::make_shared<FSDevice>();
        } else {
            return IOS::error(IOSError::NotFound);
        }

        int32_t result = device->open(path, open.mode);

        if (result < 0)
            return result;

        return allocateFileDescriptor(path, device);
    }

    case IOSCommand::Close: {
        auto it = fileDescriptors.find(request.fd);

        if (it == fileDescriptors.end())
            return IOS::error(IOSError::NotFound);

        int32_t result = it->second.device->close(request.fd);

        if (result >= 0)
            fileDescriptors.erase(it);

        return result;
    }

    case IOSCommand::Read: {
        auto it = fileDescriptors.find(request.fd);

        if (it == fileDescriptors.end())
            return IOS::error(IOSError::NotFound);

        IOSReadRequest read = parseReadRequest(request);

        return it->second.device->read(read.bufferAddress, read.size);
    }

    case IOSCommand::Write: {
        auto it = fileDescriptors.find(request.fd);

        if (it == fileDescriptors.end())
            return IOS::error(IOSError::NotFound);

        IOSWriteRequest write = parseWriteRequest(request);

        return it->second.device->write(write.bufferAddress, write.size);
    }

    case IOSCommand::Seek: {
        auto it = fileDescriptors.find(request.fd);

        if (it == fileDescriptors.end())
            return IOS::error(IOSError::NotFound);

        IOSSeekRequest seek = parseSeekRequest(request);

        return it->second.device->seek(seek.offset, seek.whence);
    }

    case IOSCommand::Ioctl: {
        auto it = fileDescriptors.find(request.fd);

        if (it == fileDescriptors.end())
            return IOS::error(IOSError::NotFound);

        IOSIoctlRequest ioctl = parseIoctlRequest(request);

        return it->second.device->ioctl(ioctl);
    }

    case IOSCommand::Ioctlv: {
        auto it = fileDescriptors.find(request.fd);

        if (it == fileDescriptors.end())
            return IOS::error(IOSError::NotFound);

        IOSIoctlvRequest ioctlv = parseIoctlvRequest(request);

        auto vectors = parseVectors(ioctlv.vectorsAddress,
                                    ioctlv.inCount + ioctlv.outCount);

        return it->second.device->ioctlv(ioctlv, vectors);
    }

    case IOSCommand::Reply:
        Logger::log("IOS", LogLevel::Warning,
                    "Unexpected IPC Reply command from PPC");
        return -1;

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

    seekRequest.offset = static_cast<int32_t>(request.args[0]);
    seekRequest.whence = request.args[1];

    return seekRequest;
}

IOSIoctlRequest IOS::parseIoctlRequest(const IOSRequest &request) {
    IOSIoctlRequest result{};

    result.ipcAddress = request.address;

    result.request = request.args[0];
    result.inPtr = request.args[1];
    result.inSize = request.args[2];
    result.outPtr = request.args[3];
    result.outSize = request.args[4];

    return result;
}

IOSIoctlvRequest IOS::parseIoctlvRequest(const IOSRequest &request) {
    IOSIoctlvRequest result{};

    result.ipcAddress = request.address;

    result.request = request.args[0];
    result.inCount = request.args[1];
    result.outCount = request.args[2];
    result.vectorsAddress = request.args[3];

    return result;
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

int32_t IOS::allocateFileDescriptor(const std::string &path,
                                    std::shared_ptr<IOSDevice> device) {
    int32_t fd = nextFileDescriptor++;

    fileDescriptors.emplace(fd, IOSFileDescriptor{
                                    .path = path,
                                    .device = std::move(device),
                                });

    return fd;
}

void IOS::prepareDiscBoot(uint64_t partitionOffset) {
    auto device = std::dynamic_pointer_cast<DIDevice>(devices.at("/dev/di"));
    if (device)
        device->prepareBoot(partitionOffset);
    auto es = std::dynamic_pointer_cast<ESDevice>(devices.at("/dev/es"));
    if (es)
        es->prepareDiscBoot(partitionOffset);
}

void IOS::completeRequest(uint32_t address, int32_t result) {
    IOSRequest request = parseRequest(address);

    Bus::writePhysical32(address + 0x04, static_cast<uint32_t>(result));

    Bus::writePhysical32(address + 0x08,
                         static_cast<uint32_t>(request.command));

    Bus::writePhysical32(address + 0x00,
                         static_cast<uint32_t>(IOSCommand::Reply));

    Device::globalDevice->ipc->replyFromStarlet(address);
}
