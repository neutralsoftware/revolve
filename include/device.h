#ifndef REVOLVE_DEVICE
#define REVOLVE_DEVICE

#include "core/memory.h"
#include "core/time.h"
#include "cpu/broadway.h"
#include "cpu/interface.h"
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

    void step();
    void start();

    Memory memory;
    MMIO mmioDispatcher;
    Broadway cpu;
    Scheduler scheduler;

    std::shared_ptr<ProcessorInterface> pi =
        std::make_shared<ProcessorInterface>();
};

#endif