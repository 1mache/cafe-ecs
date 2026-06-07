#pragma once

#include "SDL3/SDL_rect.h"
#include <bagel.h>

namespace cafe
{

/** @brief Makes an entity a child of another: its Transform follows the parent's
 *  center each frame (via hierarchySystem), offset by localOffset in screen pixels. */
struct ChildOf
{
    bagel::Entity parent{bagel::ent_type(-1)};
    SDL_FPoint    localOffset{}; // from center of parent, in screen pixels
};

} // namespace cafe

template <>
struct bagel::Storage<cafe::ChildOf> final : NoInstance
{
    using type = SparseStorage<cafe::ChildOf>;
};
