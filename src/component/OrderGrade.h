#pragma once

#include "CoffeeOverview.h"
#include "ItemTypes.h"
#include "Order.h"
#include <bagel.h>
#include <cstdint>

namespace cafe
{

inline constexpr int MAX_ITEM_GRADE     = 100;
inline constexpr int PARTIAL_ITEM_GRADE = 50;

/** @brief Tracks per-item delivery scores and served bitmask for a customer's order.
 *  Bit layout of servedMask: bit i = drinks[i] (i in 0..MAX_DRINKS-1),
 *                            bit MAX_DRINKS+j = pastries[j]. */
struct OrderGrade
{
    uint8_t servedMask{};
    int     drinkGrades[MAX_DRINKS]{};
    int     pastryGrades[MAX_PASTRIES]{};
};

inline void markDrinkServed(OrderGrade& g, int i)  { g.servedMask |= static_cast<uint8_t>(1u << i); }
inline bool isDrinkServed(const OrderGrade& g, int i) { return (g.servedMask >> i) & 1u; }

inline void markPastryServed(OrderGrade& g, int j)    { g.servedMask |= static_cast<uint8_t>(1u << (MAX_DRINKS + j)); }
inline bool isPastryServed(const OrderGrade& g, int j) { return (g.servedMask >> (MAX_DRINKS + j)) & 1u; }

/** Returns the index of the first unserved drink slot, or -1 if all are served. */
int firstUnservedDrink(const Order& order, const OrderGrade& grade);

/** Returns the index of the first unserved pastry slot, or -1 if all are served. */
int firstUnservedPastry(const Order& order, const OrderGrade& grade);

/** Returns true when every ordered drink and pastry slot has been served. */
bool allItemsServed(const Order& order, const OrderGrade& grade);

/** Returns the raw sum of all per-item grades (max = order item count * MAX_ITEM_GRADE). */
int sumItemGrades(const Order& order, const OrderGrade& grade);

/** Returns the unserved drink slot whose recipe best matches overview ratios, or -1. */
int matchDrinkSlotByRatio(const Order& order, const OrderGrade& grade, const CoffeeOverview& overview);

/** Returns the unserved pastry slot matching type/temp, or the last type-only match, or -1. */
int matchPastrySlot(const Order& order, const OrderGrade& grade,
                    PastryType pastryType, Temperature pastryTemp);

} // namespace cafe

template <> struct bagel::Storage<cafe::OrderGrade> final : bagel::NoInstance
{
    using type = bagel::SparseStorage<cafe::OrderGrade>;
};
