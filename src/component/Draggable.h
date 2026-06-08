#pragma once

#include "DropType.h"
#include <bagel.h>

namespace cafe
{
/** @brief Entity can be picked up; dropType declares which DropSpace types it is compatible with. */
struct Draggable
{
    DropType dropType{ DropType::Any };
};
} // namespace cafe

template <> struct bagel::Storage<cafe::Draggable> final : bagel::NoInstance { using type = bagel::SparseStorage<cafe::Draggable>; };
