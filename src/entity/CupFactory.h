#pragma once

#include "WorldPos.h"
#include <bagel.h>

namespace cafe
{
/** @brief Kinematic cup at @p pos. Three solid walls + one interior sensor. */
bagel::Entity createCup(WorldPos pos, int capacity = 50);

/** @brief Test-only: same as createCup but with explicit texture dimensions (skips Assets). */
bagel::Entity createCupHeadless(WorldPos pos, float texW, float texH, int capacity = 50);
} // namespace cafe
