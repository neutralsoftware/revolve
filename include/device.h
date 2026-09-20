#ifndef REVOLVE_DEVICE
#define REVOLVE_DEVICE

#include "core/memory.h"
#include <memory>
#include <stdexcept>

#define GET_DEVICE()                                                           \
    if (!Device::globalDevice) {                                               \
        throw std::runtime_error("Device not initialized");                    \
    }                                                                          \
    auto device = Device::globalDevice

class Device {
  public:
    static std::shared_ptr<Device> globalDevice;

    static std::shared_ptr<Device> createDevice();

    Memory memory;
    MMIO mmioDispatcher;
};

#endif