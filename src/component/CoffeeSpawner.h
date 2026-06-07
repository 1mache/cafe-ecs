#pragma once

#include "WorldPos.h"
#include <bagel.h>

namespace cafe
{
/** @brief Emits a stream of coffee drops while active. */
struct CoffeeSpawner
{
    float    interval{ 0.05f };  // seconds between drops
    float    accumulator{};
    bool     active{};
    WorldPos offset{};           // spawn point relative to the spawner's Transform
};
} // namespace cafe

template <> struct bagel::Storage<cafe::CoffeeSpawner> final : NoInstance { using type = SparseStorage<cafe::CoffeeSpawner>; };
