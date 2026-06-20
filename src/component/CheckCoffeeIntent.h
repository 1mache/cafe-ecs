#pragma once

#include "Ingredient.h"
#include <bagel.h>

namespace cafe
{
/** @brief Cup was dropped on a customer: expected drink spec + target customer. */
struct CheckCoffeeIntent
{
    float ratio[INGREDIENT_COUNT]{}; // Coffee, Milk, Water — normalized fractions
    bool  isHot{};                   // expected serving temperature
    int   dropSum{};                 // expected total drops poured
    int   drinkSlot{};               // which drinks[] slot on the customer this cup fulfills
    bagel::ent_type customer{ -1 };
};
} // namespace cafe

template <> struct bagel::Storage<cafe::CheckCoffeeIntent> final : bagel::NoInstance
{
    using type = bagel::SparseStorage<cafe::CheckCoffeeIntent>;
};
