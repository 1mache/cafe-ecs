#pragma once
#include <array>
#include "Sprite.h"

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
    static constexpr int           MAX_LAYERS = 2;
    std::array<Sprite, MAX_LAYERS> sprites{};
    int                            layerCount{1};
};

} // namespace cafe

template <>
struct bagel::Storage<cafe::Transform> final : NoInstance
{
    using type = SparseStorage<cafe::Transform>;
};

template <>
struct  bagel::Storage<cafe::Drawable> final : NoInstance
{
    using type = SparseStorage<cafe::Drawable>;
};