
#include "device.h"
#include "core/memory.h"
#include <memory>

std::shared_ptr<Device> Device::globalDevice = nullptr;

std::shared_ptr<Device> Device::createDevice() {
    globalDevice = std::make_shared<Device>();
    globalDevice->memory = Memory();
    globalDevice->mmioDispatcher = MMIO();
    return globalDevice;
}