#pragma once

#include "WorldPos.h"
#include <bagel.h>

namespace cafe
{
/** @brief Dynamic coffee drop. Falls under gravity, collides with cup walls, destroyed on sensor entry. */
bagel::Entity createLiquidDrop(WorldPos pos);
} // namespace cafe
