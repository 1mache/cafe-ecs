#pragma once

#include "WorldPos.h"

#include <bagel.h>

namespace cafe
{
/** @brief Straight-line drift + fade-out over the entity's Lifetime.
 *  particleSystem moves the Transform by velocity*dt each frame and scales
 *  the alpha of the entity's Drawable or TextLabel tint from 255 to 0 across
 *  its Lifetime. Colour lives in that Drawable/TextLabel tint, set at spawn. */
struct Particle
{
    WorldPos velocity{}; // world units per second
};
} // namespace cafe

template <> struct bagel::Storage<cafe::Particle> final : bagel::NoInstance
{
    using type = bagel::SparseStorage<cafe::Particle>;
};
