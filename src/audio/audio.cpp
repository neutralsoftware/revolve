#include "audio/audio.h"

#include <SDL3/SDL.h>
#include <algorithm>
#include <iostream>

AudioSystem::~AudioSystem() { shutdown(); }

bool AudioSystem::initialize() {
    if (stream) {
        return true;
    }

    SDL_AudioSpec spec{};
    spec.format = SDL_AUDIO_S16;
    spec.channels = CHANNELS;
    spec.freq = SAMPLE_RATE;

    stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec,
                                       nullptr, nullptr);

    if (!stream) {
        std::cerr << "[Audio] Failed to open playback stream: "
                  << SDL_GetError() << '\n';
        return false;
    }

    if (!SDL_ResumeAudioStreamDevice(stream)) {
        std::cerr << "[Audio] Failed to resume playback device: "
                  << SDL_GetError() << '\n';

        SDL_DestroyAudioStream(stream);
        stream = nullptr;
        return false;
    }

    std::cout << "[Audio] Host audio initialized: " << SAMPLE_RATE
              << " Hz, stereo S16\n";

    return true;
}

void AudioSystem::shutdown() {
    if (!stream) {
        return;
    }

    SDL_DestroyAudioStream(stream);
    stream = nullptr;
    pendingCount = 0;
    inputRate = SAMPLE_RATE;
}

void AudioSystem::submitSamples(std::span<const int16_t> samples,
                                uint32_t rate) {
    if (!stream || samples.empty()) {
        return;
    }

    if (rate != 32000 && rate != 48000)
        return;
    if (samples.size() % CHANNELS != 0)
        return;
    if (rate != inputRate) {
        flush();
        SDL_AudioSpec spec{SDL_AUDIO_S16, CHANNELS, static_cast<int>(rate)};
        if (!SDL_SetAudioStreamFormat(stream, &spec, nullptr))
            return;
        inputRate = rate;
    }
    while (!samples.empty()) {
        const size_t count =
            std::min(samples.size(), pendingSamples.size() - pendingCount);
        std::copy_n(samples.begin(), count,
                    pendingSamples.begin() + pendingCount);
        pendingCount += count;
        samples = samples.subspan(count);
        if (pendingCount == pendingSamples.size())
            flush();
    }
}

void AudioSystem::flush() {
    if (!stream || !pendingCount)
        return;
    if (SDL_GetAudioStreamQueued(stream) >
        static_cast<int>(inputRate * CHANNELS * sizeof(int16_t) / 4))
        SDL_ClearAudioStream(stream);
    if (!SDL_PutAudioStreamData(
            stream, pendingSamples.data(),
            static_cast<int>(pendingCount * sizeof(int16_t))))
        std::cerr << "[Audio] SDL_PutAudioStreamData failed: " << SDL_GetError()
                  << '\n';
    pendingCount = 0;
}
