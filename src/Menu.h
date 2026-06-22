#pragma once

#include "Ingredient.h"
#include "DrinkType.h"
#include "Order.h"
#include <iterator>

constexpr int name = 0;
constexpr int ratio = 1;
constexpr int allowsHot = 2;
constexpr int allowsCold = 3;
constexpr int iconFrame = 4;

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
    float       targetFill;             // ideal fill fraction (0..1) of cup capacity
};

// Indexed by DrinkType (order MUST match the enum). Each ratio sums to 1.0.
inline constexpr DrinkRecipe MENU[] = {
    // name           ratio                    hot    cold   iconFrame  targetFill
    { "Espresso",    { 1.00f, 0.00f, 0.00f }, true,  false, 6,         0.35f },
    { "Americano",   { 0.33f, 0.00f, 0.67f }, true,  true,  7,         0.85f },
    { "Cappuccino",  { 0.50f, 0.50f, 0.00f }, true,  true,  9,         0.80f },
    { "Black coffee",{ 1.00f, 0.00f, 0.00f }, true,  true,  5,         0.85f },
    { "Macchiato",   { 0.75f, 0.25f, 0.00f }, true,  false, 8,         0.45f },
};
static_assert(std::size(MENU) == static_cast<size_t>(DrinkType::count),
              "MENU must have one entry per DrinkType");

constexpr const DrinkRecipe& recipeFor(DrinkType d) { return MENU[static_cast<size_t>(d)]; }

/** @brief Builds a random order: 0-MAX_DRINKS beverages and 0-MAX_PASTRIES
 *  pastries (at least one item total), each with a random type and temperature. */
Order randomOrder();

const char* temperatureName(Temperature t);

/** @brief props.png frame index for a temperature icon (Hot = fire, Cold = ice). */
constexpr int temperatureFrame(Temperature t) { return t == Temperature::Hot ? 10 : 11; }

/** @brief Display name for a pastry type. */
const char* pastryName(PastryType p);
} // namespace cafe