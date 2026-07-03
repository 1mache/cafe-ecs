#include "Sound.h"
#include "Utils.h"

#include <iostream>
#include <utility>

namespace cafe
{

Sound::Sound(Sound&& other) noexcept
{
    std::swap(other._spec, _spec);
    std::swap(other._buf, _buf);
    std::swap(other._len, _len);
}

Sound& Sound::operator=(Sound&& other) noexcept
{
    Sound temp(std::move(other));
    std::swap(temp._spec, _spec);
    std::swap(temp._buf, _buf);
    std::swap(temp._len, _len);

    return *this;
}

bool Sound::loadFromFile(const std::string_view& path)
{
    assertFatal(!path.empty(), "Sound path is empty");

    if (!SDL_LoadWAV(path.data(), &_spec, &_buf, &_len))
    {
        std::cerr << "SDL_LoadWAV failed for " << path << ": " << SDL_GetError()
                  << "\n";
        return false;
    }

    return true;
}
} // namespace cafe
