#pragma once

#include <bagel.h>

namespace cafe
{
/** @brief Marker: entity has expired and should be destroyed this frame by cleanupSystem. */
struct Leaving {};
} // namespace cafe

template <> struct bagel::Storage<cafe::Leaving> final : NoInstance { using type = TaggedStorage<cafe::Leaving>; };
