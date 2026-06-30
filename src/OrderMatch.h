#pragma once

#include "CoffeeOverview.h"
#include "Menu.h"

namespace cafe
{

// A cup must be at least this full (0..1) before it can be served at all.
inline constexpr float MIN_SERVE_FILL = 0.80f;

/** @brief Compare a cup's beverage overview to the expected drink recipe.
 *
 *  Returns a score 0-100 from ingredient ratio, fill level, and temperature.
 *  An empty overview grades as 0. Correct cold serving adds a bonus (capped at 100);
 *  a temperature mismatch halves the score. Hot-on-hot has no temperature adjustment.
 *  Minimum fill for delivery is checked separately (see MIN_SERVE_FILL). */
int gradeDrink(const DrinkRecipe& recipe, bool expectedHot, const CoffeeOverview& overview);

} // namespace cafe
