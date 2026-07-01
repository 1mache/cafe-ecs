#pragma once

namespace cafe
{
/** @brief Read the persistent upgrade levels and stamp their effective values into
 *  the world's LiquidSpawner (interval) and Microwave (heatTime) components. Call
 *  once, after the machines/spawners are created (end of MainGameScene::onInit). */
void applyUpgradesSystem();
} // namespace cafe
