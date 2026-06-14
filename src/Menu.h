#pragma once

#include "Ingredient.h"
#include "DrinkType.h"
#include "Order.h"
#include <iterator>

namespace cafe
{
/** @brief A named drink: its ingredient ratio and which temperatures it may be served at. */
struct DrinkRecipe
{
    const char* name;
    float       ratio[INGREDIENT_COUNT]; // Coffee, Milk, Water — fractions, sum to 1.0
    bool        allowsHot;
    bool        allowsCold;
    int         iconFrame;               // index into props.png (16x16 frames)
};

// Indexed by DrinkType (order MUST match the enum). Each ratio sums to 1.0.
inline constexpr DrinkRecipe MENU[] = {
    // name           ratio                    hot    cold   iconFrame
    { "Espresso",    { 1.00f, 0.00f, 0.00f }, true,  false, 4 }, // no cold espresso
    { "Americano",   { 0.33f, 0.00f, 0.67f }, true,  true,  5 },
    { "Cappuccino",  { 0.50f, 0.50f, 0.00f }, true,  true,  7 },
    { "Black coffee",{ 1.00f, 0.00f, 0.00f }, true,  true,  3 },
    { "Macchiato",   { 0.75f, 0.25f, 0.00f }, true,  false, 6 },
};
static_assert(std::size(MENU) == static_cast<size_t>(DrinkType::count),
              "MENU must have one entry per DrinkType");

constexpr const DrinkRecipe& recipeFor(DrinkType d) { return MENU[static_cast<size_t>(d)]; }

/** @brief Builds an order that references the given drink at the given temperature. */
Order makeDrinkOrder(DrinkType d, Temperature t, bool hasPastry);

/** @brief Builds an order for a random menu drink at a random allowed temperature. */
Order randomDrinkOrder(bool hasPastry);

const char* temperatureName(Temperature t);

/** @brief props.png frame index for a temperature icon (Hot = fire, Cold = ice). */
constexpr int temperatureFrame(Temperature t) { return t == Temperature::Hot ? 8 : 9; }

/** @brief Display name for a pastry type. */
const char* pastryName(PastryType p);
} // namespace cafe