#pragma once

#include "Ingredient.h"
#include <bagel.h>

namespace cafe
{

/** @brief A client's order: a desired drink composition and/or a pastry.
 *  Invariant: hasDrink || hasPastry must be true. */
struct Order
{
    int  ratio[INGREDIENT_COUNT]{}; // desired drink mix: [coffee, milk, water]
    bool hasDrink{};
    bool hasPastry{};               // single pastry type for now
};
} // namespace cafe

template <> struct bagel::Storage<cafe::Order> final : NoInstance { using type = SparseStorage<cafe::Order>; };
