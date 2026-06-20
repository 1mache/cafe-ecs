#pragma once
#include "PhysicsFilters.h"
#include <box2d/id.h>

namespace cafe
{
/** @brief Owns the single shared Box2D world. Init once at startup, shutdown at exit. */
class PhysicsContext final
{
public:
    PhysicsContext() = default;
    PhysicsContext(const PhysicsContext&) = delete;
    PhysicsContext& operator=(const PhysicsContext&) = delete;
    PhysicsContext(PhysicsContext&&) = delete;
    PhysicsContext& operator=(PhysicsContext&&) = delete;

    void      init();
    void      cleanup();
    void      step(float dtSeconds); // fixed 1/FPS internal step
    b2WorldId world();

private:
    b2WorldId _world{ b2_nullWorldId };
    float     _accumulator{};
};
} // namespace cafe
