#pragma once

#include <SDL3/SDL_render.h>
#include <bagel.h>

namespace cafe
{
struct Drawable
{
    SDL_Texture* texture{};
    SDL_FRect    srcRect{};
    int          renderLayer{};
};
} // namespace cafe

template <> struct bagel::Storage<cafe::Drawable> final : NoInstance { using type = SparseStorage<cafe::Drawable>; };
