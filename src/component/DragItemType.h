#pragma once

#include "DropType.h"
#include <bagel.h>

namespace cafe
{
/** @brief Static category of a draggable entity; used to match DropSpace sensors. */
struct DragItemType
{
    DropType dropType{ DropType::Any };
};
} // namespace cafe

template <> struct bagel::Storage<cafe::DragItemType> final : bagel::NoInstance { using type = bagel::SparseStorage<cafe::DragItemType>; };
