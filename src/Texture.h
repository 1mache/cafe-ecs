/**
 * @file Texture.h
 * @brief RAII wrapper for SDL_Texture.
 */
#pragma once

#include <SDL3/SDL.h>

#include <string>

namespace cafe
{
/** @brief Owns an SDL_Texture; destroyed in the destructor. Non-copyable and non-movable. */
class Texture final
{
public:
    Texture(){}
    ~Texture()
    {
        if (_texture)
            SDL_DestroyTexture(_texture);
    }

    Texture(const Texture&) = delete;
    Texture(Texture&& other) noexcept;
    Texture& operator=(const Texture&) = delete;
    Texture& operator=(Texture&& other) noexcept;

    bool         loadFromFile(const std::string_view& path, SDL_Renderer* renderer);
    SDL_Texture* get() const { return _texture; }
    SDL_FPoint   getSize() const { return _size; }
    SDL_FRect    getFullSrcRect() const { return {0.f, 0.f, _size.x, _size.y}; }
private:
    SDL_Texture* _texture{};
    SDL_FPoint   _size{};
};
} // namespace megaman