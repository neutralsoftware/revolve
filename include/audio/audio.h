#ifndef REVOLVE_AUDIO
#define REVOLVE_AUDIO

#include <SDL3/SDL_audio.h>

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

    void submitSamples(std::span<const int16_t> samples);

    bool initialized() const { return stream != nullptr; }

  private:
    SDL_AudioStream *stream = nullptr;

    static constexpr int SAMPLE_RATE = 48000;
    static constexpr int CHANNELS = 2;
};

#endif