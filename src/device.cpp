
#include "device.h"
#include "SDL3/SDL_events.h"
#include "core/memory.h"
#include "core/time.h"
#include "core/utils.h"
#include "cpu/broadway.h"
#include "cpu/interface.h"
#include "cpu/memory_interface.h"
#include "graphics/video_interface.h"
#include <cstdint>
#include <memory>
#include <string>

std::shared_ptr<Device> Device::globalDevice = nullptr;

std::shared_ptr<Device> Device::createDevice() {
    globalDevice = std::make_shared<Device>();

    globalDevice->controller.setPI(globalDevice->pi.get());

    globalDevice->mmioDispatcher.registerDevice(
        PI_MMIO_BASE, PI_MMIO_END - PI_MMIO_BASE + 1, globalDevice->pi.get());
    globalDevice->mmioDispatcher.registerDevice(
        MI_MMIO_BASE, MI_MMIO_END - MI_MMIO_BASE + 1, globalDevice->mi.get());
    globalDevice->mmioDispatcher.registerDevice(
        IPC_MMIO_BASE, IPC_MMIO_END - IPC_MMIO_BASE + 1,
        globalDevice->ipc.get());
    globalDevice->mmioDispatcher.registerDevice(VI_BASE, VI_SIZE,
                                                globalDevice->vi.get());
    globalDevice->mmioDispatcher.registerDevice(CP_BASE, CP_SIZE,
                                                globalDevice->cp.get());
    globalDevice->mmioDispatcher.registerDevice(WGPIPE_BASE, WGPIPE_SIZE,
                                                globalDevice->wgpipe.get());

    globalDevice->ios.init();
    globalDevice->vi->initialize();

    globalDevice->gx.initialize();

    Bus::writePhysical32(0x0020, 0x0D15EA5E);
    Bus::writePhysical32(0x0024, 0x00000001);
    Bus::writePhysical32(0x0028, 0x01800000);
    Bus::writePhysical32(0x0034, 0x817FEC60);

    Bus::writePhysical32(0x00F0, 0x01800000);
    Bus::writePhysical32(0x00F8, 0x0E7BE2C0);
    Bus::writePhysical32(0x00FC, 0x2B73A840);

    Bus::writePhysical32(0x30D8, 0xFFFFFFFF);
    Bus::writePhysical32(0x30DC, 0);

    Bus::writePhysical8(0x315C, 0x80);

    Bus::writePhysical8(0x30E0, 0);
    Bus::writePhysical32(0x3184, 0x80000000);

    return globalDevice;
}

void Device::processIPC() {
    if (!ipc->ppcRequestPending()) {
        return;
    }

    uint32_t requestAddress = ipc->getPPCMessage();

    ipc->acknoledgeFromStarlet();

    ios.submitRequest(requestAddress);
}

void Device::step() {
    cpu.setExternalInterrupt(pi->interruptPending());

    processIPC();

    uint32_t cycles = cpu.executeInstruction();

    cpu.advanceTime(cycles);
    scheduler.advance(cycles);

    gx.run();
}

void Device::start() {
    bool running = true;

    auto window = gx.renderer->window;
    while (running) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        step();
    }
}

void Scheduler::advance(tick ticks) {
    currentTime += ticks;

    while (!eventQueue.empty() && eventQueue.top().time <= currentTime) {
        Event event = eventQueue.top();
        eventQueue.pop();
        event.callback();
    }
}
