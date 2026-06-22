#pragma once

#include "Ingredient.h"
#include <bagel.h>

namespace cafe
{
/** @brief Snapshot of a cup's beverage state, used for verification against an order. */
struct CoffeeOverview
{
    float ratio[INGREDIENT_COUNT]; // Coffee, Water, Milk — normalized fractions
    bool  isHot{};                 // actual serving temperature
    int   dropSum{};               // total drops poured
    float fillPercent{};           // dropSum / cup capacity (0..1)
};
} // namespace cafe

template <> struct bagel::Storage<cafe::CoffeeOverview> final : bagel::NoInstance
{
    using type = bagel::SparseStorage<cafe::CoffeeOverview>;
};
