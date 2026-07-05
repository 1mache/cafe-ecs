#pragma once

namespace cafe
{
class AssetManager;

/** @brief Drives the napkin's Transform toward the layout for NapkinIntent.state by
 *  (re)starting a Tween on state change. tweenSystem does the actual interpolation. */
void napkinSystem(AssetManager& assets, float dt);
} // namespace cafe
