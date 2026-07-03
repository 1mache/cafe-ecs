/**
 * @file Sound.h
 * @brief RAII wrapper for a decoded WAV buffer.
 */
#pragma once

#include <SDL3/SDL.h>

#include <string_view>

namespace cafe
{
/** @brief Owns a WAV buffer loaded via SDL_LoadWAV; freed in the destructor. Non-copyable, move-only. */
class Sound final
{
public:
    Sound() = default;
    ~Sound()
    {
        if (_buf)
            SDL_free(_buf);
    }

    Sound(const Sound&) = delete;
    Sound(Sound&& other) noexcept;
    Sound& operator=(const Sound&) = delete;
    Sound& operator=(Sound&& other) noexcept;

    bool loadFromFile(const std::string_view& path);

    [[nodiscard]]
    const SDL_AudioSpec& spec() const { return _spec; }
    [[nodiscard]]
    const Uint8*         data() const { return _buf; }
    [[nodiscard]]
    Uint32               size() const { return _len; }
private:
    SDL_AudioSpec _spec{};
    Uint8*        _buf{};
    Uint32        _len{};
};
} // namespace cafe
