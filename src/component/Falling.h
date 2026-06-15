#pragma once

#include "WorldPos.h"
#include <bagel.h>

namespace cafe
{
/** @brief Cartoonish drop-in tween: eases the item from `fromY` down to `slot`,
 *  then is removed so the item becomes draggable. Present only while animating. */
struct Falling
{
    WorldPos slot{};      // target resting position (x stays fixed, y is the landing height)
    float    fromY{};     // start height (above the screen)
    float    t{};         // elapsed seconds
    float    duration{};  // total fall time
};
} // namespace cafe

template <> struct bagel::Storage<cafe::Falling> final : bagel::NoInstance { using type = bagel::SparseStorage<cafe::Falling>; };
