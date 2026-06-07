#pragma once

#include <bagel.h>

namespace cafe
{
inline constexpr int INGREDIENT_KINDS = 3;

/** @brief Indices into Order::ratio — [Coffee=0, Milk=1, Tea=2]. */
enum class Ingredient { Coffee, Milk, Tea };

/** @brief A client's order: a desired drink composition and/or a pastry.
 *  Invariant: hasDrink || hasPastry must be true. */
struct Order
{
    int  ratio[INGREDIENT_KINDS]{}; // desired drink mix: [coffee, milk, tea]
    bool hasDrink{};
    bool hasPastry{};               // single pastry type for now
};
} // namespace cafe

template <> struct bagel::Storage<cafe::Order> final : NoInstance { using type = SparseStorage<cafe::Order>; };
