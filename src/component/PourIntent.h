#pragma once

#include <bagel.h>

namespace cafe
{
/** @brief Desired pour state for a pipe. Input writes it; pourControlSystem
 *  reads it and flips the matching CoffeeSpawner on/off next frame. */
struct PourIntent { bool active{}; };
} // namespace cafe

template <> struct bagel::Storage<cafe::PourIntent> final : NoInstance { using type = SparseStorage<cafe::PourIntent>; };
