#pragma once

#include "DropType.h"
#include <bagel.h>

namespace cafe
{
/** @brief A clickable counter button; `item` is the supply it summons when pressed.
 *  `justPressed` is set by intentSystem on a click and consumed by supplyButtonSystem. */
struct SpawnButton
{
    DropType item{ DropType::cup };
    bool     justPressed{};
};
} // namespace cafe

template <> struct bagel::Storage<cafe::SpawnButton> final : bagel::NoInstance { using type = bagel::SparseStorage<cafe::SpawnButton>; };
