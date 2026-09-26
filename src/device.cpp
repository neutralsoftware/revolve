
#include "device.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_hints.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_timer.h"
#include "core/memory.h"
#include "core/time.h"
#include "core/utils.h"
#include "cpu/broadway.h"
#include "cpu/interface.h"
#include "cpu/memory_interface.h"
#include "graphics/video_interface.h"
#include "input/gamecube.h"
#include <algorithm>
#include <cstdint>
#include <memory>
#include <numbers>
#include <string>

std::shared_ptr<Device> Device::globalDevice = nullptr;

std::shared_ptr<Device> Device::createDevice() {
    SDL_SetHint("SDL_JOYSTICK_HIDAPI_WII", "0");
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
    globalDevice->mmioDispatcher.registerDevice(DI_BASE, DI_SIZE,
                                                globalDevice->di.get());
    globalDevice->mmioDispatcher.registerDevice(0x0C006000, DI_SIZE,
                                                globalDevice->di.get());
    globalDevice->mmioDispatcher.registerDevice(0x0D806000, DI_SIZE,
                                                globalDevice->di.get());
    globalDevice->mmioDispatcher.registerDevice(SI_BASE, SI_SIZE,
                                                globalDevice->si.get());
    globalDevice->mmioDispatcher.registerDevice(EXI_BASE, EXI_SIZE,
                                                globalDevice->exi.get());

    globalDevice->mmioDispatcher.registerDevice(0x0C001000, 0x100,
                                                globalDevice->pe.get());

    globalDevice->ios.init();
    globalDevice->vi->initialize();

    globalDevice->gx.initialize();

    if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
        Logger::log("Audio", LogLevel::Warning,
                    "Failed to initialize SDL audio subsystem: " +
                        std::string(SDL_GetError()));
    }
    if (!globalDevice->audio.initialize()) {
        Logger::log("Audio", LogLevel::Warning,
                    "Failed to initialize host audio");
    }

    globalDevice->ai = std::make_shared<AudioInterface>(globalDevice->pi.get());

    globalDevice->mmioDispatcher.registerDevice(0x0D006C00, AI_SIZE,
                                                globalDevice->ai.get());
    globalDevice->mmioDispatcher.registerDevice(0x0C006C00, AI_SIZE,
                                                globalDevice->ai.get());
    globalDevice->mmioDispatcher.registerDevice(AI_BASE, AI_SIZE,
                                                globalDevice->ai.get());

    auto controller = std::make_shared<GameCubeControllerDevice>(
        &globalDevice->inputManager->gameCube(0));

    globalDevice->si->attachDevice(0, controller);

    Bus::writePhysical32(0x0020, 0x0D15EA5E);
    Bus::writePhysical32(0x0024, 0x00000001);
    Bus::writePhysical32(0x0028, 0x01800000);
    Bus::writePhysical32(0x002C, 0x00000023);
    Bus::writePhysical32(0x0030, 0);
    Bus::writePhysical32(0x0034, 0x817FEC60);

    Bus::writePhysical32(0x00E4, 0x8008F7B8);
    Bus::writePhysical32(0x00F0, 0x01800000);
    Bus::writePhysical32(0x00F4, 0x8179B500);
    Bus::writePhysical32(0x00F8, 0x0E7BE2C0);
    Bus::writePhysical32(0x00FC, 0x2B73A840);

    Bus::writePhysical32(0x0300, 0x4C000064);
    Bus::writePhysical32(0x0800, 0x4C000064);
    Bus::writePhysical32(0x0C00, 0x4C000064);

    Bus::writePhysical32(0x30C0, 0);
    Bus::writePhysical32(0x30C4, 0);

    Bus::writePhysical32(0x3100, 0x01800000);
    Bus::writePhysical32(0x3104, 0x01800000);
    Bus::writePhysical32(0x3108, 0x81800000);
    Bus::writePhysical32(0x310C, 0);
    Bus::writePhysical32(0x3110, 0x817FEC60);
    Bus::writePhysical32(0x3114, 0xDEADBEEF);
    Bus::writePhysical32(0x3118, 0x04000000);
    Bus::writePhysical32(0x311C, 0x04000000);
    Bus::writePhysical32(0x3120, 0x93400000);
    Bus::writePhysical32(0x3124, 0x90000800);
    Bus::writePhysical32(0x3128, 0x933E0000);
    Bus::writePhysical32(0x312C, 0xDEADBEEF);
    Bus::writePhysical32(0x3130, 0x933E0000);
    Bus::writePhysical32(0x3134, 0x93400000);
    Bus::writePhysical32(0x3138, 0x00000011);
    Bus::writePhysical32(0x313C, 0xDEADBEEF);
    Bus::writePhysical32(0x3148, 0x93400000);
    Bus::writePhysical32(0x314C, 0x94000000);
    Bus::writePhysical32(0x3150, 0xDEADBEEF);
    Bus::writePhysical32(0x3154, 0xDEADBEEF);
    Bus::writePhysical32(0x3158, 0x0000FF01);
    Bus::writePhysical32(0x3160, 0);

    Bus::writePhysical32(0x30D8, 0xFFFFFFFF);
    Bus::writePhysical32(0x30DC, 0);
    Bus::writePhysical16(0x30E6, 0x8201);
    Bus::writePhysical32(0x30F0, 0);

    Bus::writePhysical8(0x315C, 0x80);
    Bus::writePhysical16(0x315E, 0x0113);

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

    serviceDevices(cycles);
}

void Device::serviceDevices(uint64_t cycles) {
    constexpr uint64_t peripheralInterval = 4096;
    constexpr uint64_t iosInterval = BROADWAY_CLOCK / 1000;

    peripheralCycles += cycles;
    if (peripheralCycles >= peripheralInterval) {
        ai->step(static_cast<uint32_t>(peripheralCycles));
        dsp->step(static_cast<uint32_t>(peripheralCycles));
        si->step(peripheralCycles);
        peripheralCycles = 0;
    }

    iosCycles += cycles;
    if (iosCycles >= iosInterval) {
        ios.update();
        iosCycles = 0;
    }

    if (gx.hasPendingWork())
        gx.run();
}

void Device::runBatch(uint32_t instructionCount) {
    constexpr uint32_t serviceInterval = 1024;
    uint32_t completed = 0;

    while (completed < instructionCount && !stopRequested) {
        cpu.setExternalInterrupt(pi->interruptPending());
        processIPC();

        uint32_t count =
            std::min(serviceInterval, instructionCount - completed);
        const uint64_t eventDeadline = scheduler.ticksUntilNextEvent();
        if (eventDeadline != 0)
            count = std::min<uint32_t>(
                count, static_cast<uint32_t>(
                           std::min<uint64_t>(eventDeadline, UINT32_MAX)));
        uint64_t cycles = 0;
        uint32_t executed = 0;
        for (; executed < count; ++executed) {
            cycles += cpu.executeInstruction();
            if (cpu.isIdleLoop()) {
                ++executed;
                break;
            }
        }

        cpu.advanceTime(cycles);
        scheduler.advance(cycles);
        serviceDevices(cycles);
        completed += executed;

        fastForwardIdle();
    }
}

bool Device::fastForwardIdle() {
    if (!cpu.isIdleLoop() || pi->interruptPending())
        return false;
    const uint64_t cycles = scheduler.ticksUntilNextEvent();
    if (cycles == 0)
        return false;
    cpu.advanceTime(cycles);
    scheduler.advance(cycles);
    serviceDevices(cycles);
    return true;
}

bool Device::serviceHostEvents() {
    bool running = true;
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT)
            running = false;
    }

    SDL_PumpEvents();
    inputManager->update();
    return running;
}

void Device::start() {
    bool running = true;
    stopRequested = false;
    uint64_t nextInputPoll = 0;

    uint64_t hostEpoch = SDL_GetTicksNS();
    uint64_t guestEpoch = scheduler.now();
    while (running && !stopRequested) {
        const uint64_t hostTime = SDL_GetTicks();
        if (hostTime >= nextInputPoll) {
            nextInputPoll = hostTime + 4;
            running = serviceHostEvents();
        }

        if (!running)
            break;
        runBatch(16384);
        const uint64_t elapsedCycles = scheduler.now() - guestEpoch;
        const uint64_t guestNanoseconds =
            (elapsedCycles / BROADWAY_CLOCK) * 1000000000ULL +
            (elapsedCycles % BROADWAY_CLOCK) * 1000000000ULL / BROADWAY_CLOCK;
        const uint64_t hostNanoseconds = SDL_GetTicksNS() - hostEpoch;
        if (guestNanoseconds > hostNanoseconds + 1000000ULL)
            SDL_DelayNS(std::min<uint64_t>(guestNanoseconds - hostNanoseconds,
                                           4000000ULL));
        else if (hostNanoseconds > guestNanoseconds + 250000000ULL) {
            hostEpoch = SDL_GetTicksNS();
            guestEpoch = scheduler.now();
        }
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
