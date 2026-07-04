#pragma once

#include <bagel.h>

namespace cafe
{
/** @brief Tag on cheat sheet children (icons + ratio labels) on the full napkin. */
struct CheatSheetIcon {};
} // namespace cafe

template <> struct bagel::Storage<cafe::CheatSheetIcon> final : bagel::NoInstance
{
    using type = bagel::TaggedStorage<cafe::CheatSheetIcon>;
};
