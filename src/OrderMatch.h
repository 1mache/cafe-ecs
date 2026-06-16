#pragma once

#include "CoffeeOverview.h"
#include "Order.h"

namespace cafe
{
/** @brief Quality of a delivered drink, from its ingredient ratio vs the order. */
enum class DrinkGrade { Wrong, Acceptable, Perfect };

// A cup must be at least this full (0..1) before it can be served at all.
inline constexpr float MIN_SERVE_FILL = 0.80f;
// Max per-ingredient fraction difference allowed for each grade band.
inline constexpr float RATIO_TOL_PERFECT    = 0.05f;
inline constexpr float RATIO_TOL_ACCEPTABLE = 0.12f;

/** @brief Compare a cup's beverage overview to an order's desired drink.
 *
 *  The overview's ratio is compared to the recipe ratio for the ordered drink;
 *  the grade is decided by the largest per-ingredient absolute difference.
 *  An empty overview grades as Wrong. Fill level is NOT considered here
 *  (see MIN_SERVE_FILL, checked at delivery). */
DrinkGrade gradeDrink(const Order& order, const CoffeeOverview& overview);
} // namespace cafe
