#ifndef REVOLVE_AUDIO_INTERFACE
#define REVOLVE_AUDIO_INTERFACE

#include "core/memory.h"
#include "core/time.h"
#include "cpu/interface.h"

#include <cstdint>
#include <string>

constexpr uint32_t AI_BASE = 0x0D806C00;
constexpr uint32_t AI_SIZE = 0x20;

class AudioInterface : public MMIODevice {
  public:
    explicit AudioInterface(ProcessorInterface *pi) : pi(pi) {}

    uint32_t read(uint32_t offset, AccessSize size) override;
    void write(uint32_t offset, uint32_t value, AccessSize size) override;

    std::string getName() override { return "AudioInterface"; }

    void step(uint32_t cpuCycles);

    uint32_t sampleRate() const { return auxRate32kHz ? 48000 : 32000; }

    uint32_t dmaSampleRate() const { return rate32kHz ? 32000 : 48000; }

    uint8_t leftVolume() const { return static_cast<uint8_t>(volume & 0xFF); }

    uint8_t rightVolume() const {
        return static_cast<uint8_t>((volume >> 8) & 0xFF);
    }

  private:
    ProcessorInterface *pi = nullptr;

    // AI_CONTROL state
    bool rate32kHz = false;
    bool interruptValid = false;
    bool interruptStatus = false;
    bool interruptMask = false;
    bool auxRate32kHz = false;
    bool playing = false;

    uint16_t volume = 0;

    uint32_t sampleCounter = 0;
    uint32_t interruptTiming = 0;

    // Fractional CPU-cycle -> sample conversion.
    uint64_t sampleAccumulator = 0;

    void updateInterrupt();
    void advanceSamples(uint32_t count);

    static bool crossedCounter(uint32_t oldCounter, uint32_t newCounter,
                               uint32_t target);
};

#endif