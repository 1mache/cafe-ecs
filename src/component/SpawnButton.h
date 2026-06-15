#pragma once

#include "DropType.h"
#include <bagel.h>

namespace cafe
{
/** @brief A clickable counter button; `item` is the supply it summons when pressed. */
struct SpawnButton { DropType item{ DropType::cup }; };
} // namespace cafe

template <> struct bagel::Storage<cafe::SpawnButton> final : bagel::NoInstance { using type = bagel::SparseStorage<cafe::SpawnButton>; };
