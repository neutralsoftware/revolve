#include "core/utils.h"
#include "ios/ios.h"

int32_t DIDevice::ioctl(const IOSIoctlRequest &request) {
    Logger::log("DI", LogLevel::Warning,
                "DI not implemented yet, ioctl=0x" +
                    utils::toHexString(request.request));

    return static_cast<int32_t>(IOSError::Invalid);
}

int32_t DIDevice::ioctlv(const IOSIoctlvRequest &request,
                         const std::vector<IOSVector> &vectors) {
    Logger::log("DI", LogLevel::Warning,
                "DI not implemented yet, ioctlv=0x" +
                    utils::toHexString(request.request));

    return static_cast<int32_t>(IOSError::Invalid);
}