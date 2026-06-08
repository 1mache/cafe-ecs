#pragma once

#include "WorldPos.h"
#include <SDL3/SDL.h>
#include <bagel.h>

namespace cafe
{
/** @brief Dynamic coffee drop. Falls under gravity, collides with cup walls, destroyed on sensor entry. */
bagel::Entity createLiquidDrop(WorldPos pos, SDL_Texture* tex);
} // namespace cafe
