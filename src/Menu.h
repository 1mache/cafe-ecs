#pragma once

#include "Ingredient.h"
#include "DrinkType.h"
#include "Order.h"
#include "PastryType.h"
#include "SpriteSheet.h"
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
    float       ratio[INGREDIENT_COUNT]; // Coffee, Water, Milk — fractions, sum to 1.0
    bool        allowsHot;
    bool        allowsCold;
    int         iconFrame;               // index into props.png (16x16 frames)
    float       targetFill;             // ideal fill fraction (0..1) of cup capacity
};

// Indexed by DrinkType (order MUST match the enum). Each ratio sums to 1.0.
inline constexpr DrinkRecipe MENU[] = {
    // name           {coffee, water, milk}                    hot    cold   iconFrame  targetFill
    { "Black coffee",{ 0.80f, 0.20f, 0.00f }, true,  true,  5,         1.f },
    { "Espresso",    { 1.00f, 0.00f, 0.00f }, true,  false, 6,         0.5f },
    { "Americano",   { 0.35f, 0.50f, 0.15f }, true,  true,  7,         1.f },
    { "Latte",       { 0.20f, 0.70f, 0.10f }, true,  false, 8,         1.f },
    { "Cappuccino",  { 0.30f, 0.70f, 0.00f }, true,  true,  9,         0.75f },
};
static_assert(std::size(MENU) == static_cast<size_t>(DrinkType::count),
              "MENU must have one entry per DrinkType");

constexpr const DrinkRecipe& recipeFor(DrinkType d) { return MENU[static_cast<size_t>(d)]; }

/** @brief Builds a random order: 0-MAX_DRINKS beverages and 0-MAX_PASTRIES
 *  pastries (at least one item total), each with a random type and temperature.
 *  Built within MAX_ORDER_ICONS so it always fits the speech bubble. */
Order randomOrder();

/** @brief Validates that every PastryType and DrinkType has a sprite in props.json.
 *  json bigger than enum → SDL_Log warning (art exists, type not yet implemented).
 *  enum bigger than json → assertFatal (we'd try to draw a non-existent frame). */
void validateOrderSprites(const SpriteSheet& props);

const char* temperatureName(Temperature t);

/** @brief props.png frame index for a temperature icon (Hot = fire, Cold = ice). */
constexpr int temperatureFrame(Temperature t) { return t == Temperature::Hot ? 10 : 11; }

/** @brief Display name for a pastry type. */
const char* pastryName(PastryType p);
} // namespace cafe