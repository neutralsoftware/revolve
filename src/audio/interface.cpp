#include "audio/interface.h"

#include <cstdint>

namespace {

constexpr uint32_t CONTROL_PSTAT = 1u << 0;
constexpr uint32_t CONTROL_AFR = 1u << 1;
constexpr uint32_t CONTROL_AIINTMSK = 1u << 2;
constexpr uint32_t CONTROL_AIINT = 1u << 3;
constexpr uint32_t CONTROL_AIINTVLD = 1u << 4;
constexpr uint32_t CONTROL_SCRESET = 1u << 5;
constexpr uint32_t CONTROL_RATE = 1u << 6;

} // namespace

uint32_t AudioInterface::read(uint32_t offset, AccessSize size) {
    if (size != AccessSize::U32) {
        return 0;
    }

    switch (offset) {
    case 0x00: {
        uint32_t control = 0;

        if (playing)
            control |= CONTROL_PSTAT;

        if (stream48kHz)
            control |= CONTROL_AFR;

        if (interruptMask)
            control |= CONTROL_AIINTMSK;

        if (interruptStatus)
            control |= CONTROL_AIINT;

        if (interruptValid)
            control |= CONTROL_AIINTVLD;

        if (dma32kHz)
            control |= CONTROL_RATE;

        return control;
    }

    case 0x04:
        return volume;

    case 0x08:
        return sampleCounter;

    case 0x0C:
        return interruptTiming;

    default:
        return 0;
    }
}

void AudioInterface::write(uint32_t offset, uint32_t value, AccessSize size) {
    if (size != AccessSize::U32) {
        return;
    }

    switch (offset) {
    case 0x00: {
        if (value & CONTROL_AIINT) {
            interruptStatus = false;
        }

        if (value & CONTROL_SCRESET) {
            sampleCounter = 0;
            sampleAccumulator = 0;
        }

        playing = (value & CONTROL_PSTAT) != 0;

        const bool newStream48kHz = (value & CONTROL_AFR) != 0;
        if (stream48kHz != newStream48kHz)
            sampleAccumulator = 0;
        stream48kHz = newStream48kHz;

        interruptMask = (value & CONTROL_AIINTMSK) != 0;

        interruptValid = (value & CONTROL_AIINTVLD) != 0;

        const bool newRate32kHz = (value & CONTROL_RATE) != 0;

        if (newRate32kHz != dma32kHz) {
            dma32kHz = newRate32kHz;
        }

        updateInterrupt();
        break;
    }

    case 0x04:
        volume = static_cast<uint16_t>(value);
        break;

    case 0x08:
        sampleCounter = value;
        sampleAccumulator = 0;
        break;

    case 0x0C:
        interruptTiming = value;
        break;

    default:
        break;
    }
}

void AudioInterface::updateInterrupt() {
    if (!pi)
        return;

    if (interruptStatus && interruptMask) {
        pi->raiseInterrupt(PIInterrupt::AI);
    } else {
        pi->clearInterrupt(PIInterrupt::AI);
    }
}

void AudioInterface::step(uint32_t cpuCycles) {
    if (!playing) {
        return;
    }

    sampleAccumulator +=
        static_cast<uint64_t>(cpuCycles) * static_cast<uint64_t>(sampleRate());

    const uint32_t samples =
        static_cast<uint32_t>(sampleAccumulator / BROADWAY_CLOCK);

    sampleAccumulator %= BROADWAY_CLOCK;

    if (samples != 0) {
        advanceSamples(samples);
    }
}

bool AudioInterface::crossedCounter(uint32_t oldCounter, uint32_t newCounter,
                                    uint32_t target) {

    if (oldCounter <= newCounter) {
        return target > oldCounter && target <= newCounter;
    }

    return target > oldCounter || target <= newCounter;
}

void AudioInterface::advanceSamples(uint32_t count) {
    const uint32_t oldCounter = sampleCounter;

    sampleCounter += count;

    if (!interruptValid &&
        crossedCounter(oldCounter, sampleCounter, interruptTiming)) {

        interruptStatus = true;
    }

    updateInterrupt();
}
