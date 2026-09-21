
#include "device.h"
#include "core/memory.h"
#include "core/time.h"
#include "core/utils.h"
#include "cpu/broadway.h"
#include "cpu/interface.h"
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
        IPC_MMIO_BASE, IPC_MMIO_END - IPC_MMIO_BASE + 1,
        globalDevice->ipc.get());

    return globalDevice;
}

void Device::step() {
    cpu.setExternalInterrupt(pi->interruptPending());

    uint32_t cycles = cpu.executeInstruction();

    cpu.advanceTime(cycles);
    scheduler.advance(cycles);
}

void Device::start() {
    while (true) {
        step();
    }
}

void Scheduler::advance(tick ticks) {
    currentTime += ticks;

    while (!eventQueue.empty() && eventQueue.top().time <= currentTime) {
        Event event = eventQueue.top();
        eventQueue.pop();
        Logger::log("Scheduler", LogLevel::Info,
                    "Executed event: " + event.name + " at time " +
                        std::to_string(event.time));
        event.callback();
    }
}