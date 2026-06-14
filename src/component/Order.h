#pragma once

#include "DrinkType.h"
#include <bagel.h>

namespace cafe
{

/** @brief A client's order: a menu drink (its ratio lives on the recipe) and/or a pastry.
 *  Invariant: hasDrink || hasPastry must be true. */
struct Order
{
    DrinkType   drink{ DrinkType::Espresso }; // which menu drink (ratio comes from recipeFor)
    Temperature temperature{ Temperature::Hot };
    bool        hasDrink{};
    bool        hasPastry{};                   // single pastry type for now
};
} // namespace cafe

template <> struct bagel::Storage<cafe::Order> final : NoInstance { using type = SparseStorage<cafe::Order>; };
