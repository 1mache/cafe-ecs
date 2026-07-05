#pragma once

#include <SDL3/SDL.h>

namespace cafe
{
class AssetManager;
class PhysicsContext;
class AudioContext;

/** @brief Spawns drops from every active LiquidSpawner, each of its own kind.
 *  Also drives the milk-steamer sound while a milk pipe is pouring. */
void liquidSpawnerSystem(AssetManager& assets, PhysicsContext& physics, AudioContext& audio, float dtSeconds);

void liquidVelocityClampSystem();
} // namespace cafe
