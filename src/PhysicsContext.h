#pragma once
#include "GameConfig.h"
#include <box2d/id.h>

namespace cafe
{
/** @brief Owns the single shared Box2D world. Init once at startup, shutdown at exit. */
class PhysicsContext
{
public:
    static void      init();
    static void      shutdown();
    static void      step(float dtSeconds); // fixed 1/FPS internal step
    static b2WorldId world();

private:
    static inline b2WorldId _world{ b2_nullWorldId };
    static inline float     _accumulator{};
};
} // namespace cafe
