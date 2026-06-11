#pragma once

#include "Cup.h"
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

/** @brief Compare a cup's composition to an order's desired ratio.
 *
 *  Both order.ratio and cup.filled are normalized to fractions of their totals;
 *  the grade is decided by the largest per-ingredient absolute difference.
 *  An empty order or an empty cup grades as Wrong. Fill level is NOT considered
 *  here (see MIN_SERVE_FILL, checked at delivery). */
DrinkGrade gradeDrinkRatio(const Order& order, const Cup& cup);
} // namespace cafe
