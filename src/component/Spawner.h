#pragma once

#include "WorldPos.h"
#include <bagel.h>

namespace cafe
{
/** @brief Drives the customer cycle: spawns one customer at a time at `seat`,
 *  waiting `interval` seconds after the seat empties before the next spawn. */
struct Spawner
{
    WorldPos seat;      // where the spawned customer appears
    float    patience;  // patience given to each spawned customer
    float    interval;  // delay after the seat empties before the next spawn
    float    cooldown;  // seconds remaining; ticks down only while the seat is empty
};
} // namespace cafe

template <> struct bagel::Storage<cafe::Spawner> final : NoInstance { using type = SparseStorage<cafe::Spawner>; };
