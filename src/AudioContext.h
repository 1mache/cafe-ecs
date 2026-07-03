#pragma once
#include "Sound.h"

#include <SDL3/SDL.h>

#include <array>
#include <string>
#include <string_view>
#include <unordered_map>

namespace cafe
{
/** @brief Owns the audio device and a pool of mixing voices. Init once at startup, shutdown at exit. */
class AudioContext final
{
public:
    AudioContext() = default;
    AudioContext(const AudioContext&) = delete;
    AudioContext& operator=(const AudioContext&) = delete;
    AudioContext(AudioContext&&) = delete;
    AudioContext& operator=(AudioContext&&) = delete;

    void init();
    void cleanup();

    // Lazy-load (from res/sounds/) and fire-and-forget a WAV. Dropped if all voices are busy.
    void play(std::string_view filename);

    // Hold-to-play sounds (e.g. a steamer): startSustained on press, stopSustained on release.
    // A single dedicated channel — starting a different sound replaces the one already sustaining.
    // startOffsetSeconds skips that far into the file (e.g. past a slow fade-in).
    void startSustained(std::string_view filename, float startOffsetSeconds = 0.f);
    void stopSustained();

    // Master output gain (1.0 = unchanged).
    void setVolume(float gain);
private:
    const Sound& getSound(std::string_view filename);

    static constexpr int  VOICES       = 8;
    static constexpr auto RES_DIR_PATH = "res/sounds/";

    SDL_AudioDeviceID                      _device{};
    std::array<SDL_AudioStream*, VOICES>   _voices{};
    SDL_AudioStream*                       _sustainedVoice{};
    std::string                            _sustainedName{}; // empty = nothing sustaining
    std::unordered_map<std::string, Sound> _sounds{};
};
} // namespace cafe