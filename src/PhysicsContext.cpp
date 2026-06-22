#include "PhysicsContext.h"
#include "Utils.h"
#include "GameConfig.h"
#include <box2d/box2d.h>

namespace cafe
{
void PhysicsContext::init()
{
    assertFatal(!b2World_IsValid(_world), "PhysicsContext::init called twice");
    b2WorldDef wd = b2DefaultWorldDef();
    wd.gravity = { 0.f, -GRAVITY }; // world is Y-up; gravity pulls down
    _world = b2CreateWorld(&wd);
    _accumulator = 0.f;
}

void PhysicsContext::cleanup()
{
    if (b2World_IsValid(_world))
    {
        b2DestroyWorld(_world);
        _world = b2_nullWorldId;
    }
    _accumulator = 0.f;
}

void PhysicsContext::step(float dtSeconds)
{
    // Fixed timestep with a leftover accumulator — keeps physics stable even
    // if a frame stalls. subSteps is Box2D's internal solver iteration count.
    constexpr float fixedDt  = 1.f / FPS;
    constexpr int   subSteps = 8;
    constexpr float maxDt    = 0.25f; // spiral-of-death guard for very long stalls

    // Box2D clears its sensor-event buffer at the start of every b2World_Step, so
    // accumulate each sub-step's events here for systems to read once per frame.
    _beginEvents.clear();
    _endEvents.clear();

    if (dtSeconds > maxDt) dtSeconds = maxDt;
    _accumulator += dtSeconds;
    while (_accumulator >= fixedDt)
    {
        b2World_Step(_world, fixedDt, subSteps);

        const b2SensorEvents ev = b2World_GetSensorEvents(_world);
        _beginEvents.insert(_beginEvents.end(),
                            ev.beginEvents, ev.beginEvents + ev.beginCount);
        _endEvents.insert(_endEvents.end(),
                          ev.endEvents, ev.endEvents + ev.endCount);

        _accumulator -= fixedDt;
    }
}

b2WorldId PhysicsContext::world()
{
    return _world;
}
} // namespace cafe
