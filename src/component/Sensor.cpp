#include "Sensor.h"
#include "PhysicsBody.h"
#include <box2d/box2d.h>
#include <vector>

namespace cafe
{
namespace
{
void setBodySensorEvents(b2BodyId body, bool enabled)
{
    if (!b2Body_IsValid(body)) return;

    const int count = b2Body_GetShapeCount(body);
    if (count <= 0) return;

    std::vector<b2ShapeId> shapes(static_cast<size_t>(count));
    b2Body_GetShapes(body, shapes.data(), count);

    for (b2ShapeId shapeId : shapes)
    {
        if (b2Shape_IsValid(shapeId) && b2Shape_IsSensor(shapeId))
            b2Shape_EnableSensorEvents(shapeId, enabled);
    }
}
} // namespace

void enableSensor(bagel::ent_type ent)
{
    const bagel::Entity e{ ent };
    if (!e.has<PhysicsBody>()) return;

    setBodySensorEvents(e.get<PhysicsBody>().id, true);
    if (!e.has<Sensor>())
        e.add(Sensor{});
}

void disableSensor(bagel::ent_type ent)
{
    const bagel::Entity e{ ent };

    if (e.has<PhysicsBody>())
        setBodySensorEvents(e.get<PhysicsBody>().id, false);

    if (e.has<Sensor>())
        e.del<Sensor>();
}

} // namespace cafe
