#include "core/utils.h"
#include "ios/ios.h"

int32_t ESDevice::ioctlv(const IOSIoctlvRequest &request,
                         const std::vector<IOSVector> &vectors) {
    Logger::log("ES", LogLevel::Warning,
                "Unimplemented ES ioctlv 0x" +
                    utils::toHexString(request.request));

    return static_cast<int32_t>(IOSError::Invalid);
}

int32_t ESDevice::open(const std::string &path, uint32_t mode) {
    Logger::log("ES", LogLevel::Warning,
                "Unimplemented ES open for path: " + path);

    return static_cast<int32_t>(IOSError::Invalid);
}

int32_t ESDevice::close(int32_t fd) {
    Logger::log("ES", LogLevel::Warning,
                "Unimplemented ES close for fd: " + std::to_string(fd));

    return static_cast<int32_t>(IOSError::Invalid);
}