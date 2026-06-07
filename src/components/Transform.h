#pragma once

#include <bagel.h>

namespace cafe
{

/** @brief World-space AABB transform: center (x,y), half-extents (w,h), rotation. */
struct Transform
{
    // In world coordinates
    float x{}, y{}; // center
    float w{}, h{}; // half-extents from center, like box2d
    // degrees or radians, convertible at callsite
    float rot{};
};

} // namespace cafe

template <>
struct bagel::Storage<cafe::Transform> final : NoInstance
{
    using type = SparseStorage<cafe::Transform>;
};
