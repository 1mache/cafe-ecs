#pragma once

#include <bagel.h>

namespace cafe
{
class AssetManager;

/** @brief Static napkin sprite anchored to the bottom-center of the screen. */
bagel::Entity createNapkin(AssetManager& assets);
} // namespace cafe
