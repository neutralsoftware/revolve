#ifndef REVOLVE_DEVICE
#define REVOLVE_DEVICE

#include "core/memory.h"
#include "core/system_hardware.h"
#include "core/time.h"
#include "cpu/broadway.h"
#include "cpu/interface.h"
#include "cpu/memory_interface.h"
#include "disc.h"
#include "graphics/cp.h"
#include "graphics/gx.h"
#include "graphics/video_interface.h"
#include "graphics/wgpipe.h"
#include "input/gamecube.h"
#include "input/manager.h"
#include "input/serial.h"
#include "ios/ios.h"
#include "ios/ipc.h"
#include <memory>
#include <stdexcept>

#define GET_DEVICE()                                                           \
    if (!Device::globalDevice) {                                               \
        throw std::runtime_error("Device not initialized");                    \
    }                                                                          \
    auto device = Device::globalDevice

enum class HollywoodIRQ : uint32_t {
    IPC = 30,
};

class HollywoodInterruptController {
  public:
    inline void raise(HollywoodIRQ irq) {
        flags |= 1u << static_cast<uint32_t>(irq);
        updatePI();
    }

    inline void clear(HollywoodIRQ irq) {
        flags &= ~(1u << static_cast<uint32_t>(irq));
        updatePI();
    }

    uint32_t getFlags() const { return flags; }
    uint32_t getMask() const { return mask; }
    void acknowledge(uint32_t value) {
        flags &= ~value;
        updatePI();
    }

    inline void setMask(uint32_t value) {
        mask = value;
        updatePI();
    }

    inline void setPI(ProcessorInterface *piDevice) { pi = piDevice; }

  private:
    uint32_t flags = 0;
    uint32_t mask = 1u << static_cast<uint32_t>(HollywoodIRQ::IPC);

    ProcessorInterface *pi = nullptr;

    inline void updatePI() {
        if (flags & mask)
            pi->raiseInterrupt(PIInterrupt::Hollywood);
        else
            pi->clearInterrupt(PIInterrupt::Hollywood);
    }
};

class Device {
  public:
    static std::shared_ptr<Device> globalDevice;

    static std::shared_ptr<Device> createDevice();

    void step();
    void start();

    void processIPC();

    Memory memory;
    MMIO mmioDispatcher;
    Broadway cpu;
    Scheduler scheduler;
    HollywoodInterruptController controller;
    GX gx;

    IOS ios;

    std::shared_ptr<ProcessorInterface> pi =
        std::make_shared<ProcessorInterface>();
    std::shared_ptr<MemoryInterface> mi = std::make_shared<MemoryInterface>();
    std::shared_ptr<IPC> ipc = std::make_shared<IPC>();
    std::shared_ptr<DiscImage> disc = std::make_shared<DiscImage>();
    std::shared_ptr<VideoInterface> vi = std::make_shared<VideoInterface>();
    std::shared_ptr<PixelEngine> pe = std::make_shared<PixelEngine>();
    std::shared_ptr<CommandProcessor> cp = std::make_shared<CommandProcessor>();
    std::shared_ptr<WriteGatherPipe> wgpipe =
        std::make_shared<WriteGatherPipe>();
    std::shared_ptr<DSPInterface> dsp = std::make_shared<DSPInterface>();
    std::shared_ptr<SerialInterface> si = std::make_shared<SerialInterface>();
    std::shared_ptr<ExpansionInterface> exi =
        std::make_shared<ExpansionInterface>();
    std::shared_ptr<InputManager> inputManager =
        std::make_shared<InputManager>();
};

#endif
