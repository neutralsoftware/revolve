#include "ios/ios.h"
#include <cstdint>

int32_t FSDevice::open(const std::string &path, uint32_t mode) { return -1; }

int32_t FSDevice::close(int32_t fd) { return -1; }

int32_t FSDevice::read(uint32_t buffer, uint32_t size) { return -1; }

int32_t FSDevice::write(uint32_t buffer, uint32_t size) { return -1; }

int32_t FSDevice::seek(int32_t offset, uint32_t whence) { return -1; }

int32_t FSDevice::ioctl(const IOSIoctlRequest &request) { return -1; }

int32_t FSDevice::ioctlv(const IOSIoctlvRequest &request,
                         const std::vector<IOSVector> &vectors) {
    return -1;
}