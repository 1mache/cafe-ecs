#pragma once

#include <bagel.h>

namespace cafe
{
/** @brief State for the pastry TV's rotating display. Lives on the pastry-icon
 *  entity itself (which also carries the Drawable being cycled). The current
 *  pastry type is static_cast<PastryType>(index); enum order is the queue. */
struct PastryTv
{
    int   index{ 0 };   // current slot in the queue (enum order)
    float timer{ 0.f }; // seconds spent on the current pastry
};

inline constexpr auto  PASTRY_TV_TEX         = "pastry_tv.png";
inline constexpr float PASTRY_TV_ROTATE_TIME = 5.f; // seconds per pastry
} // namespace cafe

template <> struct bagel::Storage<cafe::PastryTv> final : bagel::NoInstance
{
    using type = bagel::SparseStorage<cafe::PastryTv>;
};
