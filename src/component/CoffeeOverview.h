#pragma once

#include <bagel.h>

namespace cafe
{
/** @brief Snapshot of a cup's poured ingredient totals, used for beverage verification. */
struct CoffeeOverview
{
    int milkSum{};
    int waterSum{};
    int coffeeSum{};
};
} // namespace cafe

template <> struct bagel::Storage<cafe::CoffeeOverview> final : bagel::NoInstance
{
    using type = bagel::SparseStorage<cafe::CoffeeOverview>;
};
