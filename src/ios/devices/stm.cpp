#include "device.h"
#include "ios/ios.h"

IOSResult STMImmediateDevice::ioctl(const IOSIoctlRequest &request) {
    switch (static_cast<STMIoctl>(request.request)) {
    case STMIoctl::Shutdown:
    case STMIoctl::Idle:
        Device::globalDevice->requestStop();
        return 0;
    case STMIoctl::ReleaseEH:
        Device::globalDevice->ios.releaseSTMEventHook();
        return 0;
    case STMIoctl::VideoDimming:
    case STMIoctl::LEDFlash:
    case STMIoctl::LEDMode:
        return 0;
    default:
        return IOS::error(IOSError::Invalid);
    }
}

IOSResult STMEventHookDevice::ioctl(const IOSIoctlRequest &request) {
    if (request.request != static_cast<uint32_t>(STMIoctl::EventHook) || request.outSize < 4)
        return IOS::error(IOSError::Invalid);
    if (pendingRequest)
        return IOS::error(IOSError::Exists);
    pendingRequest = request.ipcAddress;
    eventOutput = request.outPtr;
    return std::nullopt;
}

void STMEventHookDevice::complete(uint32_t event) {
    if (!pendingRequest)
        return;
    const auto address = *pendingRequest;
    pendingRequest.reset();
    Bus::writePhysical32(eventOutput, event);
    eventOutput = 0;
    Device::globalDevice->ios.completeRequest(address, 0);
}

void STMEventHookDevice::triggerReset() { complete(0x00020000); }
void STMEventHookDevice::triggerPower() { complete(0x00000800); }
void STMEventHookDevice::release() { complete(0); }
int32_t STMEventHookDevice::close(int32_t) {
    release();
    return 0;
}
