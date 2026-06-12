#include "DragAndDropSystem.h"
#include "Components.h"
#include "PhysicsContext.h"
#include "PhysicsFilters.h"
#include "RenderContext.h"
#include "Transform.h"
#include <bagel.h>
#include <box2d/box2d.h>
#include <vector>

namespace cafe
{
namespace
{
bagel::ent_type entityIdFromBody(b2BodyId body)
{
    return bagel::ent_type{
        static_cast<int>(reinterpret_cast<uintptr_t>(b2Body_GetUserData(body)))
    };
}

void setBodySensorEvents(b2BodyId body, bool enabled)
{
    if (!b2Body_IsValid(body)) return;
    const int count = b2Body_GetShapeCount(body);
    if (count <= 0) return;
    std::vector<b2ShapeId> shapes(static_cast<size_t>(count));
    b2Body_GetShapes(body, shapes.data(), count);
    for (b2ShapeId shapeId : shapes)
        b2Shape_EnableSensorEvents(shapeId, enabled);
}

void enableBodySensorEventsIfDisabled(b2BodyId body)
{
    if (!b2Body_IsValid(body)) return;
    const int count = b2Body_GetShapeCount(body);
    if (count <= 0) return;
    std::vector<b2ShapeId> shapes(static_cast<size_t>(count));
    b2Body_GetShapes(body, shapes.data(), count);
    for (b2ShapeId shapeId : shapes)
    {
        if (!b2Shape_AreSensorEventsEnabled(shapeId))
            b2Shape_EnableSensorEvents(shapeId, true);
    }
}

// Moves a held entity (and its body) to follow the mouse in world space.
void holdFollow(bagel::Entity e, const DragIntent& intent)
{
    const WorldPos worldPos =
        screenToWorldPoint(intent.mousePos, RenderContext::getCameraPos());

    auto& t = e.get<Transform>();
    t.x     = worldPos.x;
    t.y     = worldPos.y;

    if (e.has<PhysicsBody>())
    {
        const b2BodyId body = e.get<PhysicsBody>().id;
        if (b2Body_IsValid(body))
            b2Body_SetTransform(body, { worldPos.x, worldPos.y }, b2Body_GetRotation(body));
    }
}

// Snaps a released entity onto its drop space (or restores gravity), disables
// sensor events, and resets the intent back to None.
void releaseEntity(bagel::Entity e, DragIntent& intent)
{
    if (intent.dropSpaceEntity.has_value())
    {
        bagel::Entity dropSpace{ *intent.dropSpaceEntity };

        if (dropSpace.has<Transform>())
        {
            const auto& dst = dropSpace.get<Transform>();
            auto&       src = e.get<Transform>();
            src.x           = dst.x;
            src.y           = dst.y;
        }

        if (e.has<PhysicsBody>())
        {
            const b2BodyId body = e.get<PhysicsBody>().id;
            if (b2Body_IsValid(body))
            {
                const auto& t = e.get<Transform>();
                b2Body_SetTransform(body, { t.x, t.y }, b2Body_GetRotation(body));
                b2Body_SetGravityScale(body, 0.f);
            }
        }

        if (dropSpace.has<PhysicsBody>())
            setBodySensorEvents(dropSpace.get<PhysicsBody>().id, false);
    }
    else if (e.has<PhysicsBody>())
    {
        const b2BodyId body = e.get<PhysicsBody>().id;
        if (b2Body_IsValid(body))
        {
            b2Body_SetGravityScale(body, 1.f);
            setBodySensorEvents(body, false);
        }
    }

    intent.intentType      = DragIntentType::None;
    intent.dropSpaceEntity = std::nullopt;
}

// Re-arms all DropSpace sensors so begin/end contacts fire while dragging.
void enableDropSpaceSensors()
{
    static const bagel::Mask dropSpaceMask =
        bagel::MaskBuilder().set<DropSpace>().set<PhysicsBody>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (e.test(dropSpaceMask))
            enableBodySensorEventsIfDisabled(e.get<PhysicsBody>().id);
    }
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

void dragAndDropSystem()
{
    static const bagel::Mask mask =
        bagel::MaskBuilder().set<DragIntent>().set<Transform>().build();

    bool anyHeld = false;

    // Single pass: branch per entity on its drag state. Field mutation only
    // (no add/del), so iterating while mutating is safe.
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask)) continue;

        auto& intent = e.get<DragIntent>();
        switch (intent.intentType)
        {
        case DragIntentType::held:
            anyHeld = true;
            if (e.has<PhysicsBody>())
                setBodySensorEvents(e.get<PhysicsBody>().id, true);
            holdFollow(e, intent);
            break;

        case DragIntentType::released:
            releaseEntity(e, intent);
            break;

        case DragIntentType::None:
            break;
        }
    }

    // Re-arm DropSpace sensors only while dragging, so they stay disabled
    // after a drop until the next drag begins.
    if (anyHeld)
        enableDropSpaceSensors();
}

void dropSpaceDetectionSystem()
{
    static const bagel::Mask heldMask      = bagel::MaskBuilder().set<DragIntent>().build();
    static const bagel::Mask dropSpaceMask = bagel::MaskBuilder().set<DropSpace>().build();

    const b2SensorEvents events = b2World_GetSensorEvents(PhysicsContext::world());

    for (int i = 0; i < events.beginCount; ++i)
    {
        const auto& be = events.beginEvents[i];
        if (!b2Shape_IsValid(be.visitorShapeId)) continue;
        if (!b2Shape_IsValid(be.sensorShapeId))  continue;

        const b2BodyId      visitorBody = b2Shape_GetBody(be.visitorShapeId);
        const bagel::Entity visitor     { entityIdFromBody(visitorBody) };
        if (!visitor.test(heldMask)) continue;
        if (visitor.get<DragIntent>().intentType != DragIntentType::held) continue;

        const b2BodyId      sensorBody = b2Shape_GetBody(be.sensorShapeId);
        const bagel::Entity sensor     { entityIdFromBody(sensorBody) };
        if (!sensor.test(dropSpaceMask)) continue;

        auto& intent = visitor.get<DragIntent>();
        if (sensor.get<DropSpace>().dropType == intent.dropType)
            intent.dropSpaceEntity = sensor.entity();
    }

    for (int i = 0; i < events.endCount; ++i)
    {
        const auto& ee = events.endEvents[i];
        if (!b2Shape_IsValid(ee.visitorShapeId)) continue;
        if (!b2Shape_IsValid(ee.sensorShapeId))  continue;

        const b2BodyId      visitorBody = b2Shape_GetBody(ee.visitorShapeId);
        const bagel::Entity visitor     { entityIdFromBody(visitorBody) };
        if (!visitor.test(heldMask)) continue;

        const bagel::ent_type sensorId = entityIdFromBody(b2Shape_GetBody(ee.sensorShapeId));

        auto& intent = visitor.get<DragIntent>();
        if (intent.dropSpaceEntity.has_value() && intent.dropSpaceEntity->id == sensorId.id)
            intent.dropSpaceEntity = std::nullopt;
    }
}

} // namespace cafe
