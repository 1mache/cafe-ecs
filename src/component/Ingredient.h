#pragma once

#include <SDL3/SDL.h>
#include <cstddef>

namespace cafe
{
/** @brief Drink ingredients. Also indexes Order::ratio and Cup::filled.
 *  `count` is a sentinel (must stay last) — its value is the number of real
 *  ingredients, used to size arrays and bound loops. */
enum class LiquidIngredient { Coffee = 0, Water, Milk, count };
static constexpr size_t INGREDIENT_COUNT = static_cast<size_t>(LiquidIngredient::count);

inline constexpr SDL_Color liquidColors[INGREDIENT_COUNT] = {
    { 102,  57,  49,  255 }, // Coffee
    { 138, 235, 241, 127 }, // Water
    { 240, 234, 214, 255 }, // Milk
};
constexpr SDL_Color liquidColor(LiquidIngredient liquid)
{
    // defensive. just in case
    if (liquid == LiquidIngredient::count) return {255, 255, 255, 255};
    return liquidColors[static_cast<size_t>(liquid)];
}
inline constexpr SDL_Color ICE_COLOR = { 120, 200, 255, 255 };
} // namespace cafe