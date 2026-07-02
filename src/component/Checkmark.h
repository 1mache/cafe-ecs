#pragma once

#include <bagel.h>

namespace cafe
{
/** @brief A hidden checkmark overlay sitting on one order icon in the bubble.
 *  Identifies the order slot it belongs to; revealed once that slot is served.
 *  The owning customer is found by walking the ChildOf chain
 *  (checkmark -> bubble -> customer). */
struct Checkmark
{
    bool isDrink{};
    int  slot{};
};
} // namespace cafe

template <> struct bagel::Storage<cafe::Checkmark> final : bagel::NoInstance
{
    using type = bagel::SparseStorage<cafe::Checkmark>;
};