#pragma once

#include <bagel.h>

namespace cafe
{
/** @brief Marker: entity (and anything that references it via ChildOf.parent
 *  or Liquid/Ice.holdingContainer) should be destroyed this frame. Set by any
 *  system that decides an entity dies; consumed once by destroySystem, which
 *  runs last. See destroySystem for the closure + destroy passes. */
struct Destroy {};
} // namespace cafe

template <> struct bagel::Storage<cafe::Destroy> final : NoInstance { using type = TaggedStorage<cafe::Destroy>; };
