#pragma once

#include "Ingredient.h"
#include <bagel.h>

namespace cafe
{
/** @brief A liquid drop. `kind` is which ingredient this drop is made of.
 *  `owner` is the cup it was counted into (-1 until it lands), so a delivered
 *  cup's drops can be destroyed without touching any other cup's contents. */
struct Liquid
{
    Ingredient      kind{ Ingredient::Coffee };
    bagel::ent_type holdingContainer{ -1 };
};
} // namespace cafe

template <> struct bagel::Storage<cafe::Liquid> final : NoInstance { using type = SparseStorage<cafe::Liquid>; };