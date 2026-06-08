#pragma once

#include "DropType.h"
#include <bagel.h>
#include <optional>

namespace cafe
{
/** @brief Added to an entity while it is actively being dragged by the player. */
struct Held
{
    DropType                    dropType{ DropType::Any };
    std::optional<bagel::ent_type> dropSpaceEntity{ std::nullopt };
};
} // namespace cafe

template <> struct bagel::Storage<cafe::Held> final : bagel::NoInstance { using type = bagel::SparseStorage<cafe::Held>; };
