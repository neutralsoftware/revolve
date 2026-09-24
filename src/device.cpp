
#include "device.h"
#include "SDL3/SDL_events.h"
#include "core/memory.h"
#include "core/time.h"
#include "core/utils.h"
#include "cpu/broadway.h"
#include "cpu/interface.h"
#include "cpu/memory_interface.h"
#include "graphics/video_interface.h"
#include "input/gamecube.h"
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
    globalDevice->mmioDispatcher.registerDevice(
        0x0D000000, IPC_MMIO_END - IPC_MMIO_BASE + 1, globalDevice->ipc.get());
    globalDevice->mmioDispatcher.registerDevice(VI_BASE, VI_SIZE,
                                                globalDevice->vi.get());
    globalDevice->mmioDispatcher.registerDevice(CP_BASE, CP_SIZE,
                                                globalDevice->cp.get());
    globalDevice->mmioDispatcher.registerDevice(WGPIPE_BASE, WGPIPE_SIZE,
                                                globalDevice->wgpipe.get());
    globalDevice->mmioDispatcher.registerDevice(DSP_BASE, DSP_SIZE,
                                                globalDevice->dsp.get());
    globalDevice->mmioDispatcher.registerDevice(SI_BASE, SI_SIZE,
                                                globalDevice->si.get());
    globalDevice->mmioDispatcher.registerDevice(EXI_BASE, EXI_SIZE,
                                                globalDevice->exi.get());

    globalDevice->mmioDispatcher.registerDevice(0x0C001000, 0x100,
                                                globalDevice->pe.get());

    globalDevice->ios.init();
    globalDevice->vi->initialize();

    globalDevice->gx.initialize();

    auto controller = std::make_shared<GameCubeControllerDevice>(
        &globalDevice->inputManager->gameCube(0));

    globalDevice->si->attachDevice(0, controller);

    Bus::writePhysical32(0x0020, 0x0D15EA5E);
    Bus::writePhysical32(0x0024, 0x00000001);
    Bus::writePhysical32(0x0028, 0x01800000);
    Bus::writePhysical32(0x0034, 0x817FEC60);

    Bus::writePhysical32(0x00F0, 0x01800000);
    Bus::writePhysical32(0x00F8, 0x0E7BE2C0);
    Bus::writePhysical32(0x00FC, 0x2B73A840);

    Bus::writePhysical32(0x3100, 0x01800000);
    Bus::writePhysical32(0x3104, 0x01800000);
    Bus::writePhysical32(0x3108, 0x81800000);
    Bus::writePhysical32(0x310C, 0);
    Bus::writePhysical32(0x3110, 0x817FEC60);
    Bus::writePhysical32(0x3118, 0x04000000);
    Bus::writePhysical32(0x311C, 0x04000000);
    Bus::writePhysical32(0x3120, 0x93400000);
    Bus::writePhysical32(0x3124, 0x90000800);
    Bus::writePhysical32(0x3128, 0x933E0000);
    Bus::writePhysical32(0x3130, 0x933E0000);
    Bus::writePhysical32(0x3134, 0x93400000);
    Bus::writePhysical32(0x3148, 0x93400000);
    Bus::writePhysical32(0x314C, 0x94000000);

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

        SDL_PumpEvents();

        inputManager->update();

        if (!running)
            break;
        for (uint32_t instruction = 0; instruction < 4096; ++instruction)
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
