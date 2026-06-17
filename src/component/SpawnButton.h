#pragma once

#include "DropType.h"
#include "WorldPos.h"
#include <bagel.h>
#include <box2d/id.h>

namespace cafe
{
/** @brief A clickable counter button; `item` is the supply it summons when pressed.
 *  `justPressed` is set by intentSystem on a click and consumed by supplyButtonSystem.
 *  `slotSensor` is queried each press to skip spawn when the slot is occupied. */
struct SpawnButton
{
    DropType  item{ DropType::Cup };
    b2ShapeId spawnSlotSensor{};
    WorldPos  spawnPos{};
    bool      justPressed{};
};
} // namespace cafe

template <> struct bagel::Storage<cafe::SpawnButton> final : NoInstance { using type = SparseStorage<cafe::SpawnButton>; };
