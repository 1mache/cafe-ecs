#pragma once

#include <bagel.h>

namespace cafe
{
/** @brief Auto-destroys the entity after `duration` seconds. Aged and
 *  enforced by lifetimeSystem, which destroys via destroyPhysicalEntity
 *  (the idempotent free-list-safe path). */
struct Lifetime
{
    float age{0.f};
    float duration{1.f};
};
} // namespace cafe

template <> struct bagel::Storage<cafe::Lifetime> final : bagel::NoInstance
{
    using type = bagel::SparseStorage<cafe::Lifetime>;
};
