#pragma once

#include "ItemTypes.h"

namespace cafe
{
class AssetManager;

/** Advances the pastry TV: every PASTRY_TV_ROTATE_TIME seconds it shows the next
 *  pastry type in the queue (enum order, looping). */
void pastryTvSystem(AssetManager& assets, float dt);

/** The pastry type currently shown on the TV, or PastryType::count if there is
 *  no TV (which createPastry treats as "pick a random type"). */
PastryType currentTvPastryType();
} // namespace cafe