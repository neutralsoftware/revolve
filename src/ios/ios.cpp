#include "ios/ios.h"
#include "core/memory.h"

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
    case IOSCommand::Open:
        Logger::log("IOS", LogLevel::Info, "OPEN request");
        break;

    case IOSCommand::Close:
        Logger::log("IOS", LogLevel::Info, "CLOSE request");
        break;

    case IOSCommand::Read:
        Logger::log("IOS", LogLevel::Info, "READ request");
        break;

    case IOSCommand::Write:
        Logger::log("IOS", LogLevel::Info, "WRITE request");
        break;

    case IOSCommand::Seek:
        Logger::log("IOS", LogLevel::Info, "SEEK request");
        break;

    case IOSCommand::Ioctl:
        Logger::log("IOS", LogLevel::Info, "IOCTL request");
        break;

    case IOSCommand::Ioctlv:
        Logger::log("IOS", LogLevel::Info, "IOCTLV request");
        break;

    default:
        Logger::log("IOS", LogLevel::Warning, "Unknown IOS command");
        break;
    }

    return -1;
}