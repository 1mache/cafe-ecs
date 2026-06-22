#pragma once

namespace cafe
{
class AssetManager;
class PhysicsContext;

/** @brief Microwave mechanic. Must run AFTER deliverySystem() and BEFORE
 *  dragAndDropSystem(), so it consumes a released pat's DragIntent before the
 *  drag system would snap it onto the machine.
 *  Intake: a pat released over a free microwave is absorbed (destroyed) and the
 *  timer starts; a pat released over a busy microwave is rejected (falls).
 *  Cook: each busy microwave ticks dt; at HEAT_TIME it spits out the same
 *  PastryType with temperature = HEATED_TEMPERATURE. */
void microwaveSystem(AssetManager& assets, PhysicsContext& physics, float dt);
} // namespace cafe
