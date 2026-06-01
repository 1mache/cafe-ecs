#pragma once
#include "SDL3/SDL_render.h"


#include <bagel.h>
namespace cafe
{
// Definition of all components and their storage

/** @brief World-space AABB transform: center (x,y), half-extents (w,h), rotation. */
struct Transform
{
    // *In world coordinates
    float x{}, y{}; // center
    float w{}, h{}; // from the center. like box2d
    // degrees or radians but can be converted into another type later
    float rot{};
};

struct Drawable
{
    SDL_Texture* texture{};
    SDL_FRect    srcRect{};
};

} // namespace cafe

template <>
struct bagel::Storage<cafe::Transform> final : bagel::NoInstance
{
    using type = bagel::SparseStorage<cafe::Transform>;
};

template <>
struct  bagel::Storage<cafe::Drawable> final : bagel::NoInstance
{
    using type = bagel::SparseStorage<cafe::Drawable>;
};