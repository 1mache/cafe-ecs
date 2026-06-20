#pragma once

#include "../DropType.h"
#include <bagel.h>

namespace cafe
{
/** @brief Marks an entity as a valid landing zone for dragged entities of the matching DropType. */
struct DropSpace
{
    DropType dropType{ DropType::Any };
};
} // namespace cafe

template <> struct bagel::Storage<cafe::DropSpace> final : bagel::NoInstance { using type = bagel::SparseStorage<cafe::DropSpace>; };
