#pragma once

#include "AssetManager.h"
#include "DropType.h"
#include "WorldPos.h"
#include <bagel.h>

namespace cafe
{
/** @brief Creates a clickable counter button that summons `item` when pressed. */
bagel::Entity createSpawnButton(AssetManager& assets, WorldPos pos, DropType item);
} // namespace cafe
