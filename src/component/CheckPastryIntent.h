#pragma once

#include "ItemTypes.h"
#include "Pastry.h"
#include <bagel.h>

namespace cafe
{
/** @brief Pastry was dropped on a customer: expected type + temperature + target customer. */
struct CheckPastryIntent
{
    PastryType      type{};         // expected pastry type from order slot
    Temperature     temp{};         // expected serving temperature
    int             pastrySlot{};   // which pastries[] slot on the customer this fulfills
    bagel::ent_type customer{ -1 };
};
} // namespace cafe

template <> struct bagel::Storage<cafe::CheckPastryIntent> final : NoInstance
{
    using type = SparseStorage<cafe::CheckPastryIntent>;
};
