#pragma once

#include <bagel.h>

namespace cafe
{
/** @brief Marker: cup was dropped on a customer and awaits beverage verification. */
struct CheckCoffeeIntent {};
} // namespace cafe

template <> struct bagel::Storage<cafe::CheckCoffeeIntent> final : bagel::NoInstance
{
    using type = bagel::TaggedStorage<cafe::CheckCoffeeIntent>;
};
