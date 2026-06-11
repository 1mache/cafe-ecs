#pragma once

#include <cstddef>

namespace cafe
{
/** @brief Drink ingredients. Also indexes Order::ratio and Cup::filled.
 *  `count` is a sentinel (must stay last) — its value is the number of real
 *  ingredients, used to size arrays and bound loops. */
enum class Ingredient { Coffee = 0, Milk, Water, count };
static constexpr size_t INGREDIENT_COUNT = static_cast<size_t>(Ingredient::count);
} // namespace cafe