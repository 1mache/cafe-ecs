#pragma once

#include <bagel.h>

namespace cafe
{
/** @brief Tag — marks an entity as a coffee drop. */
struct Liquid {};
} // namespace cafe

template <> struct bagel::Storage<cafe::Liquid> final : NoInstance { using type = SparseStorage<cafe::Liquid>; };
