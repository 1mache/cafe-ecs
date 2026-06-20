#pragma once

#include <bagel.h>

namespace cafe
{
/** @brief An ice cube. holdingContainer is the cup it landed in (-1 until it
 *  lands), so a delivered cup's ice can be destroyed without touching any other
 *  cup's contents. Mirrors Liquid's tagging field. Ice is intentionally NOT a
 *  Liquid, so it stays out of the beverage ratio/dropSum math. */
struct Ice
{
    bagel::ent_type holdingContainer{ -1 };
};
} // namespace cafe

template <> struct bagel::Storage<cafe::Ice> final : NoInstance { using type = SparseStorage<cafe::Ice>; };
