#pragma once

#include <bagel.h>

namespace cafe
{
/** @brief Tag — marks an entity as a Box2D sensor source for overlap gathering. */
struct Sensor {};

void enableSensor(bagel::ent_type ent);
void disableSensor(bagel::ent_type ent);

} // namespace cafe

template <> struct bagel::Storage<cafe::Sensor> final : bagel::NoInstance { using type = bagel::TaggedStorage<cafe::Sensor>; };
