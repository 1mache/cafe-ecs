#pragma once

#include "WorldPos.h"
#include <bagel.h>

namespace cafe
{
class AssetManager;

/** @brief Placeholder gray-square ice machine, a pure visual anchor (no
 *  collider). The spawn button placed on it drops ice cubes from the spout. */
bagel::Entity createIceMachine(AssetManager& assets, WorldPos pos);
} // namespace cafe
