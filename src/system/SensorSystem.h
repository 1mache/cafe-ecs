#pragma once

namespace cafe
{
class PhysicsContext;

/** @brief Polls every Sensor-tagged body's shapes for current overlaps
 *  (b2Shape_GetSensorOverlaps) and keeps InSensorArea up to date accordingly. */
void sensorGatheringSystem();

/** @brief Drains Box2D begin-touch sensor events (the event-buffer PhysicsContext
 *  accumulates, not the overlap-polling sensorGatheringSystem does): counts cup
 *  fills (liquid/ice) and tags anything that touched the cleanup zone (liquid,
 *  ice, cup, pastry) Destroy. */
void sensorEventSystem(PhysicsContext& physics);
} // namespace cafe
