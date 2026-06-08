#include "DragAndDropSystem.h"
#include "Components.h"
#include "PhysicsContext.h"
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

bool hitTestEntityAtWorld(bagel::Entity e, const WorldPos& worldPos, const b2Vec2& b2Pos)
{
    if (e.has<PhysicsBody>())
    {
        const b2BodyId body = e.get<PhysicsBody>().id;
        if (!b2Body_IsValid(body)) return false;

        const int count = b2Body_GetShapeCount(body);
        std::vector<b2ShapeId> shapes(static_cast<size_t>(count));
        b2Body_GetShapes(body, shapes.data(), count);
        for (b2ShapeId shapeId : shapes)
        {
            if (b2Shape_TestPoint(shapeId, b2Pos))
                return true;
        }
        return false;
    }

    if (e.has<Transform>())
    {
        const auto& t = e.get<Transform>();
        return worldPos.x >= t.x - t.w && worldPos.x <= t.x + t.w &&
               worldPos.y >= t.y - t.h && worldPos.y <= t.y + t.h;
    }

    return false;
}
} // namespace

void dropSpacePickupSystem(SDL_FPoint screenPos)
{
    static const bagel::Mask mask =
        bagel::MaskBuilder().set<DropSpace>().set<Transform>().build();

    const WorldPos worldPos = screenToWorldPoint(screenPos, RenderContext::getCameraPos());
    const b2Vec2   b2Pos    = { worldPos.x, worldPos.y };

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask)) continue;
        if (!hitTestEntityAtWorld(e, worldPos, b2Pos)) continue;
        if (!e.has<PhysicsBody>()) continue;

        enableBodySensorEventsIfDisabled(e.get<PhysicsBody>().id);
    }
}

void dragStartSystem(SDL_FPoint screenPos)
{
    static const bagel::Mask pickMask =
        bagel::MaskBuilder().set<Draggable>().set<Transform>().build();

    const WorldPos worldPos = screenToWorldPoint(screenPos, RenderContext::getCameraPos());
    const b2Vec2   b2Pos    = { worldPos.x, worldPos.y };

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(pickMask)) continue;
        if (e.has<Held>()) continue;
        if (!hitTestEntityAtWorld(e, worldPos, b2Pos)) continue;

        const DropType dt = e.get<Draggable>().dropType;
        e.add(Held{ .dropType = dt });
        return;
    }
}

void midDragSystem(SDL_FPoint screenPos)
{
    static const bagel::Mask heldMask =
        bagel::MaskBuilder().set<Held>().set<Transform>().build();

    const WorldPos worldPos = screenToWorldPoint(screenPos, RenderContext::getCameraPos());

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(heldMask)) continue;
        

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
}

void dropSpaceDetectionSystem()
{
    static const bagel::Mask heldMask      = bagel::MaskBuilder().set<Held>().build();
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

        const b2BodyId      sensorBody = b2Shape_GetBody(be.sensorShapeId);
        const bagel::Entity sensor     { entityIdFromBody(sensorBody) };
        if (!sensor.test(dropSpaceMask)) continue;

        auto& held = visitor.get<Held>();
        if (sensor.get<DropSpace>().dropType == held.dropType)
            held.dropSpaceEntity = sensor.entity();
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

        auto& held = visitor.get<Held>();
        if (held.dropSpaceEntity.has_value() && held.dropSpaceEntity->id == sensorId.id)
            held.dropSpaceEntity = std::nullopt;
    }
}

void dragReleaseSystem()
{
    static const bagel::Mask releaseMask =
        bagel::MaskBuilder().set<Held>().set<Transform>().build();

    // Collect first — del<Held>() must not happen mid-iteration.
    std::vector<bagel::ent_type> toRelease;
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (e.test(releaseMask))
            toRelease.push_back(e.entity());
    }

    for (const bagel::ent_type id : toRelease)
    {
        bagel::Entity e{ id };
        const auto&   held = e.get<Held>();

        if (held.dropSpaceEntity.has_value())
        {
            bagel::Entity dropSpace{ *held.dropSpaceEntity };

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
                    if (e.has<Transform>())
                    {
                        const auto& t = e.get<Transform>();
                        b2Body_SetTransform(body, { t.x, t.y }, b2Body_GetRotation(body));
                    }
                    b2Body_SetGravityScale(body, 0.f);
                }
            }

            if (dropSpace.has<PhysicsBody>())
            {
                const b2BodyId dsBody = dropSpace.get<PhysicsBody>().id;
                setBodySensorEvents(dsBody, false);
            }
        }
        else
        {
            if (e.has<PhysicsBody>())
            {
                const b2BodyId body = e.get<PhysicsBody>().id;
                if (b2Body_IsValid(body))
                {
                    b2Body_SetGravityScale(body, 1.f);
                    setBodySensorEvents(body, false);
                }
            }
        }

        e.del<Held>();
    }
}

} // namespace cafe
