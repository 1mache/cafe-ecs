#include "Texture.h"
#include "Utils.h"

#include <SDL3_image/SDL_image.h>

#include <iostream>

namespace cafe
{

Texture::Texture(Texture&& other) noexcept
{
    std::swap(other._texture, _texture);
    std::swap(other._size, _size);
}

Texture& Texture::operator=(Texture&& other) noexcept
{
    Texture temp(std::move(other));
    std::swap(other._texture, _texture);
    std::swap(other._size, _size);

    return *this;
}
bool Texture::loadFromFile(const std::string_view& path, SDL_Renderer* renderer)
{
    assertFatal(renderer != nullptr, "Renderer is null");
    assertFatal(!path.empty(), "Texture path is empty");

    _texture = IMG_LoadTexture(renderer, path.data());
    if (!_texture)
    {
        std::cerr << "IMG_LoadTexture failed for " << path << ": " << SDL_GetError()
                  << "\n";
        return false;
    }

    float w{}, h{};
    SDL_GetTextureSize(_texture, &w, &h);
    _size = {w, h};

    return true;
}
} // namespace megaman