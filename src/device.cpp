
#include "device.h"
#include "core/memory.h"
#include "core/time.h"
#include "core/utils.h"
#include "cpu/broadway.h"
#include "cpu/interface.h"
#include <cstdint>
#include <memory>

std::shared_ptr<Device> Device::globalDevice = nullptr;

std::shared_ptr<Device> Device::createDevice() {
    globalDevice = std::make_shared<Device>();

    globalDevice->mmioDispatcher.registerDevice(
        PI_MMIO_BASE, PI_MMIO_END - PI_MMIO_BASE + 1, globalDevice->pi.get());

    return globalDevice;
}

void Device::step() {
    cpu.setExternalInterrupt(pi->interruptPending());

    uint32_t cycles = cpu.executeInstruction();

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
        event.callback();
        Logger::log("Scheduler", LogLevel::Info,
                    "Executed event: " + event.name + " at time " +
                        utils::toHexString(event.time));
    }
}