#include "core/utils.h"
#include "device.h"
#include "ios/ios.h"
#include <cstdint>

IOSResult STMImmediateDevice::ioctl(const IOSIoctlRequest &request) {
    switch (request.request) {
    case 0x2001: // HotReset
        Logger::log("STM", LogLevel::Info, "HotReset requested");
        Device::globalDevice->start();
        return 0;

    case 0x2003: // Shutdown
    case 0x2004: // Reset
        Logger::log("STM", LogLevel::Info, "Shutdown requested");
        exit(0);
        return 0;

    case 0x5001:
    case 0x6002:
        Logger::log("STM", LogLevel::Info,
                    "Ignoring unimplemented STM ioctl: 0x" +
                        utils::toHexString(request.request));
        return 0;

    default:
        Logger::log("STM", LogLevel::Warning,
                    "Unknown STM ioctl: 0x" +
                        utils::toHexString(request.request));
        return -1;
    }
}

IOSResult STMEventHookDevice::ioctl(const IOSIoctlRequest &request) {
    switch (request.request) {
    case 0x1000: // EventHook
        Logger::log("STM", LogLevel::Info, "EventHook requested");
        pendingRequest = request.inPtr;
        return 0;

    default:
        Logger::log("STM", LogLevel::Warning,
                    "Unknown STM ioctl: 0x" +
                        utils::toHexString(request.request));
        return -1;
    }
}

void STMEventHookDevice::triggerReset() {
    if (pendingRequest) {
        uint32_t requestAddress = *pendingRequest;
        Logger::log("STM", LogLevel::Info,
                    "Triggering reset event for request @ 0x" +
                        utils::toHexString(requestAddress));
        Bus::writePhysical32(requestAddress + 0x04, 0);
        Bus::writePhysical32(requestAddress + 0x08,
                             static_cast<uint32_t>(IOSCommand::Reply));
        Bus::writePhysical32(requestAddress + 0x00,
                             static_cast<uint32_t>(IOSCommand::Reply));
        Device::globalDevice->ipc->replyFromStarlet(requestAddress);
        pendingRequest.reset();
    }
}

void STMEventHookDevice::triggerPower() {
    if (pendingRequest) {
        uint32_t requestAddress = *pendingRequest;
        Logger::log("STM", LogLevel::Info,
                    "Triggering power event for request @ 0x" +
                        utils::toHexString(requestAddress));
        Bus::writePhysical32(requestAddress + 0x04, 0);
        Bus::writePhysical32(requestAddress + 0x08,
                             static_cast<uint32_t>(IOSCommand::Reply));
        Bus::writePhysical32(requestAddress + 0x00,
                             static_cast<uint32_t>(IOSCommand::Reply));
        Device::globalDevice->ipc->replyFromStarlet(requestAddress);
        pendingRequest.reset();
    }
}