#include "OrderMatch.h"
#include <algorithm>
#include <cmath>

namespace cafe
{
DrinkGrade gradeDrinkRatio(const Order& /*order*/, const Cup& /*cup*/)
{
    // TODO: re-implement against the menu — the desired ratio now lives on the drink
    // recipe (recipeFor(order.drink).ratio), since Order no longer carries `ratio[]`.
    // Stubbed to Perfect so the project builds and the serve flow stays testable.
    return DrinkGrade::Perfect;

    /*
    int orderTotal = 0;
    int cupTotal   = 0;
    for (size_t i = 0; i < INGREDIENT_COUNT; ++i)
    {
        orderTotal += order.ratio[i];
        cupTotal   += cup.filled[i];
    }
    if (orderTotal == 0 || cupTotal == 0)
        return DrinkGrade::Wrong;

    float maxDiff = 0.f;
    for (size_t i = 0; i < INGREDIENT_COUNT; ++i)
    {
        const float want = static_cast<float>(order.ratio[i]) / static_cast<float>(orderTotal);
        const float got  = static_cast<float>(cup.filled[i])  / static_cast<float>(cupTotal);
        maxDiff = std::max(maxDiff, std::fabs(want - got));
    }

    if (maxDiff <= RATIO_TOL_PERFECT)    return DrinkGrade::Perfect;
    if (maxDiff <= RATIO_TOL_ACCEPTABLE) return DrinkGrade::Acceptable;
    return DrinkGrade::Wrong;
    */
}
} // namespace cafe