#pragma once

#include <bagel.h>

namespace cafe
{
/** @brief Tag — marks the out-of-bounds cleanup sensor. */
struct CleanupZone {};
} // namespace cafe

template <> struct bagel::Storage<cafe::CleanupZone> final : NoInstance { using type = TaggedStorage<cafe::CleanupZone>; };
