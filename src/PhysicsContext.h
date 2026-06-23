#pragma once
#include "PhysicsFilters.h"
#include <box2d/id.h>
#include <box2d/types.h>
#include <span>
#include <vector>

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

    // Sensor events accumulated across every internal sub-step of the last step().
    // Box2D clears its own event buffer each b2World_Step, so reading it once after
    // the fixed-step loop would miss all but the final sub-step. Read these instead.
    std::span<const b2SensorBeginTouchEvent> sensorBeginEvents() const { return _beginEvents; }
    std::span<const b2SensorEndTouchEvent>   sensorEndEvents()   const { return _endEvents; }

private:
    b2WorldId _world{ b2_nullWorldId };
    float     _accumulator{};

    std::vector<b2SensorBeginTouchEvent> _beginEvents;
    std::vector<b2SensorEndTouchEvent>   _endEvents;
};
} // namespace cafe
