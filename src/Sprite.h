#pragma once
#include "SDL3/SDL_render.h"

namespace cafe
{
    struct Sprite
    {
        SDL_Texture* texture{};
        SDL_FRect    srcRect{};
        int          renderLayer{};
    };
}