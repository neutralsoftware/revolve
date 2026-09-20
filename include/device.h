#ifndef REVOLVE_DEVICE
#define REVOLVE_DEVICE

#include "core/memory.h"
#include <memory>

class Device {
  public:
    static std::shared_ptr<Device> globalDevice;

    static std::shared_ptr<Device> createDevice();

    Memory memory;
    MMIO mmioDispatcher;
};

#endif