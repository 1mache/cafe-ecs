#pragma once

#include "WorldPos.h"

namespace cafe
{
class AssetManager;

/** Creates the pastry TV in the world: a static frame sprite plus a pastry-icon
 *  entity on top that pastryTvSystem cycles through the pastry types. */
void createPastryTv(AssetManager& assets, WorldPos pos);
} // namespace cafe