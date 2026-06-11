#include "Draggable.h"
#include "Components.h"
#include "PhysicsFilters.h"
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
    for (b2ShapeId s : shapes)
        b2Shape_EnableSensorEvents(s, enabled);
}
} // namespace

void addDraggableVisitorShape(b2BodyId body, float halfW, float halfH)
{
    b2ShapeDef visitor = b2DefaultShapeDef();
    visitor.isSensor            = true;
    visitor.filter.categoryBits = filter::DRAGGABLE;
    visitor.filter.maskBits     = filter::MASK_DRAGGABLE;
    visitor.enableSensorEvents  = true;
    b2Polygon box = b2MakeOffsetBox(halfW, halfH, { 0.f, 0.f }, b2Rot_identity);
    b2CreatePolygonShape(body, &visitor, &box);
}

void enableSensorEventsOnHeldEntities()
{
    static const bagel::Mask heldMask = bagel::MaskBuilder().set<Held>().build();
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(heldMask)) continue;
        if (!e.has<PhysicsBody>()) continue;
        setBodySensorEvents(e.get<PhysicsBody>().id, true);
    }
}

} // namespace cafe
