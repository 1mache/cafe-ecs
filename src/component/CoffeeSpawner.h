#pragma once

#include "Ingredient.h"
#include "WorldPos.h"
#include <bagel.h>

namespace cafe
{
/** @brief Emits a stream of drops of one ingredient while active. */
struct LiquidSpawner
{
    Ingredient kind{ Ingredient::Coffee }; // which liquid this pipe pours
    float    interval{ 0.05f };  // seconds between drops
    float    accumulator{};
    bool     active{};           // runtime "currently pouring" flag (set by intentSystem)
    WorldPos offset{};           // spawn point relative to the spawner's Transform
};
} // namespace cafe

template <> struct bagel::Storage<cafe::LiquidSpawner> final : NoInstance { using type = SparseStorage<cafe::LiquidSpawner>; };
