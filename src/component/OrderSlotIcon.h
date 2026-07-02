#pragma once

namespace cafe
{
/** @brief Tags a bubble order icon with the order slot it represents, so a
 *  checkmark can be placed on it once that slot is served. The owning customer
 *  is found by walking the ChildOf chain (icon -> bubble -> customer). */
struct OrderSlotIcon
{
    bool isDrink{};
    int  slot{};
};
} // namespace cafe

#include <bagel.h>

template <> struct bagel::Storage<cafe::OrderSlotIcon> final : bagel::NoInstance
{
    using type = bagel::SparseStorage<cafe::OrderSlotIcon>;
};