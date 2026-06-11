#pragma once

#include "Ingredient.h"
#include <bagel.h>

namespace cafe
{
/** @brief A liquid drop. `kind` is which ingredient this drop is made of. */
struct Liquid { Ingredient kind{ Ingredient::Coffee }; };
} // namespace cafe

template <> struct bagel::Storage<cafe::Liquid> final : NoInstance { using type = SparseStorage<cafe::Liquid>; };