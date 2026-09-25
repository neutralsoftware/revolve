#ifndef REVOLVE_AUDIO
#define REVOLVE_AUDIO

#include <SDL3/SDL_audio.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

class AudioSystem {
  public:
    AudioSystem() = default;
    ~AudioSystem();

    bool initialize();
    void shutdown();

    void submitSamples(std::span<const int16_t> samples, uint32_t rate = 48000);

    bool initialized() const { return stream != nullptr; }

  private:
    SDL_AudioStream *stream = nullptr;

    uint32_t inputRate = 48000;
    std::array<int16_t, 512> pendingSamples{};
    size_t pendingCount = 0;
    void flush();

    static constexpr int SAMPLE_RATE = 48000;
    static constexpr int CHANNELS = 2;
};

#endif