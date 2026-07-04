#pragma once

#include <bagel.h>

namespace cafe
{
/** @brief Tag on drink icons shown on the full napkin cheat sheet. */
struct CheatSheetIcon {};
} // namespace cafe

template <> struct bagel::Storage<cafe::CheatSheetIcon> final : bagel::NoInstance
{
    using type = bagel::TaggedStorage<cafe::CheatSheetIcon>;
};
