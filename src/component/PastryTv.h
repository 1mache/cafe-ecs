#pragma once

#include "ItemTypes.h"
#include <bagel.h>

namespace cafe
{
/** @brief State for the pastry TV's rotating display. Lives on the pastry-icon
 *  entity itself (which also carries the Drawable being cycled). The queue is
 *  shuffled at creation (createPastryTv), so each day/new game shows a different
 *  random line; index walks the queue. */
struct PastryTv
{
    PastryType queue[static_cast<int>(PastryType::count)]{}; // the day's line
    int   index{ 0 };   // current slot in the queue
    float timer{ 0.f }; // seconds spent on the current pastry
};

inline constexpr auto  PASTRY_TV_TEX         = "pastry_tv.png";
inline constexpr float PASTRY_TV_ROTATE_TIME = 3.5f; // seconds per pastry
} // namespace cafe

template <> struct bagel::Storage<cafe::PastryTv> final : bagel::NoInstance
{
    using type = bagel::SparseStorage<cafe::PastryTv>;
};
