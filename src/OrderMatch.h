#pragma once

#include "CheckCoffeeIntent.h"
#include "CoffeeOverview.h"

namespace cafe
{

// A cup must be at least this full (0..1) before it can be served at all.
inline constexpr float MIN_SERVE_FILL = 0.80f;

/** @brief Compare a cup's beverage overview to the expected drink on CheckCoffeeIntent.
 *
 *  Returns a score 0-100: 100 = perfect ratio match, 0 = completely wrong.
 *  An empty overview grades as 0. Fill level is NOT considered here
 *  (see MIN_SERVE_FILL, checked at delivery). */
int gradeDrink(const CheckCoffeeIntent& intent, const CoffeeOverview& overview);

} // namespace cafe
