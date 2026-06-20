#pragma once

#include "DrinkType.h"
#include "PastryType.h"
#include <bagel.h>

namespace cafe
{

inline constexpr int MAX_DRINKS   = 3;
inline constexpr int MAX_PASTRIES = 3;
// Max order icons drawn in the speech bubble; orders are trimmed to fit this.
inline constexpr int MAX_ORDER_ICONS = 5;

/** @brief One ordered beverage: which drink and at what temperature. */
struct DrinkItem
{
    DrinkType   type{ DrinkType::Espresso }; // ratio comes from recipeFor(type)
    Temperature temp{ Temperature::Hot };
};

/** @brief One ordered pastry: which pastry and at what temperature. */
struct PastryItem
{
    PastryType  type{ PastryType::Croissant }; // value == props.png frame
    Temperature temp{ Temperature::Hot };
};

/** @brief A client's order: up to MAX_DRINKS beverages and MAX_PASTRIES pastries.
 *  Invariant: hasDrink || hasPastry must be true (at least one item). */
struct Order
{
    DrinkItem  drinks[MAX_DRINKS];
    int        drinkCount{};
    PastryItem pastries[MAX_PASTRIES];
    int        pastryCount{};
    bool       hasDrink{};   // == (drinkCount  > 0); kept for existing grading readers
    bool       hasPastry{};  // == (pastryCount > 0)
};
} // namespace cafe

template <> struct bagel::Storage<cafe::Order> final : NoInstance { using type = SparseStorage<cafe::Order>; };
