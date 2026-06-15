#pragma once

#include <SDL3/SDL.h>
#include <cstddef>

namespace cafe
{
/** @brief Drink ingredients. Also indexes Order::ratio and Cup::filled.
 *  `count` is a sentinel (must stay last) — its value is the number of real
 *  ingredients, used to size arrays and bound loops. */
enum class Ingredient { Coffee = 0, Milk, Water, count };
static constexpr size_t INGREDIENT_COUNT = static_cast<size_t>(Ingredient::count);

inline constexpr SDL_Color ingredientColors[INGREDIENT_COUNT] = {
    { 102,  57,  49,  255 }, // Coffee
    { 240, 234, 214, 255 }, // Milk
    { 138, 235, 241, 127 }, // Water
};
} // namespace cafe