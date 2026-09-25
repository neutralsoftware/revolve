#include "audio/audio.h"

#include <SDL3/SDL.h>
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
}

void AudioSystem::submitSamples(std::span<const int16_t> samples) {
    if (!stream || samples.empty()) {
        return;
    }

    const int byteCount = static_cast<int>(samples.size_bytes());

    if (!SDL_PutAudioStreamData(stream, samples.data(), byteCount)) {

        std::cerr << "[Audio] SDL_PutAudioStreamData failed: " << SDL_GetError()
                  << '\n';
    }
}