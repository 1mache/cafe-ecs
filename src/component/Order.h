#pragma once

#include "ItemTypes.h"
#include "Pastry.h"
#include <bagel.h>

namespace cafe
{

inline constexpr int MAX_DRINKS      = 3;
inline constexpr int MAX_PASTRIES    = 3;
inline constexpr int MAX_ORDER_ICONS = 5;

/** @brief A client's order: up to MAX_DRINKS beverages and MAX_PASTRIES pastries.
 *  Invariant: hasDrink || hasPastry must be true (at least one item). */
struct Order
{
    struct DrinkItem
    {
        DrinkType   type{ DrinkType::Espresso };
        Temperature temp{ Temperature::Hot };
    };

    struct PastryItem
    {
        PastryType  type{ PastryType::Croissant };
        Temperature temp{ Temperature::Hot };
    };

    DrinkItem  drinks[MAX_DRINKS];
    int        drinkCount{};
    PastryItem pastries[MAX_PASTRIES];
    int        pastryCount{};
    bool       hasDrink{};
    bool       hasPastry{};
};
} // namespace cafe

template <> struct bagel::Storage<cafe::Order> final : NoInstance { using type = SparseStorage<cafe::Order>; };
