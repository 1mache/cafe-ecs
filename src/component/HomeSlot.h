#pragma once
#include "WorldPos.h"
#include <bagel.h>

namespace cafe
{
/** @brief The resting slot a draggable item returns to after a delivery (its supply spot). */
struct HomeSlot
{
    WorldPos pos;
};
} // namespace cafe

template <> struct bagel::Storage<cafe::HomeSlot> final : bagel::NoInstance { using type = bagel::SparseStorage<cafe::HomeSlot>; };
