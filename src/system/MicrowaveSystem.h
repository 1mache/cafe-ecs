#pragma once

namespace cafe
{
class AssetManager;
class PhysicsContext;
class AudioContext;

/** @brief Microwave mechanic. Must run AFTER deliverySystem() and BEFORE
 *  dragAndDropSystem(), so it consumes a released pat's DragIntent before the
 *  drag system would snap it onto the machine.
 *  Intake: a pat released over a free microwave is absorbed (destroyed) and the
 *  timer starts; a pat released over a busy microwave is rejected (falls).
 *  If dropSpaceEntity is unset (no sensor begin), intake falls back to
 *  boxesOverlap against microwave transforms.
 *  Cook: each busy microwave ticks dt; at heatTime it spits out the same
 *  PastryType with temperature = HEATED_TEMPERATURE. */
void microwaveSystem(AssetManager& assets, PhysicsContext& physics, AudioContext& audio, float dt);

} // namespace cafe
