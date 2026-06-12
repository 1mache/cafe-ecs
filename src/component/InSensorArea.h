#pragma once

#include <bagel.h>

namespace cafe
{
/** @brief Entity id of the Sensor currently overlapping this entity. */
struct InSensorArea
{
    bagel::ent_type sensor{ -1 };
};
} // namespace cafe

template <> struct bagel::Storage<cafe::InSensorArea> final : bagel::NoInstance { using type = bagel::SparseStorage<cafe::InSensorArea>; };
