#pragma once

#include <bagel.h>
#include <box2d/id.h>

namespace cafe
{
/** @brief Handle to a Box2D body. Always destroy via destroyPhysicalEntity. */
struct PhysicsBody
{
    b2BodyId id{ b2_nullBodyId };
};
} // namespace cafe

template <> struct bagel::Storage<cafe::PhysicsBody> final : NoInstance { using type = SparseStorage<cafe::PhysicsBody>; };
