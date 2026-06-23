#include "DragAndDropSystem.h"
#include "Components.h"
#include "PhysicsContext.h"
#include "PhysicsFilters.h"
#include "RenderContext.h"
#include "Transform.h"
#include <bagel.h>
#include <box2d/box2d.h>
#include <algorithm>
#include <optional>

namespace cafe
{
namespace
{
constexpr float MAX_FOLLOW_SPEED    = 25.f; // m/s the held body chases the cursor
constexpr float ARRIVE_THRESHOLD    = 0.f; // m; within this, stop (deadzone)
constexpr float ARRIVE_THRESHOLD_SQ = ARRIVE_THRESHOLD * ARRIVE_THRESHOLD;

float speedByDist(float dist)
{
    return std::min(dist*dist, MAX_FOLLOW_SPEED);
}

bagel::ent_type entityIdFromBody(b2BodyId body)
{
    return bagel::ent_type{
        static_cast<int>(reinterpret_cast<uintptr_t>(b2Body_GetUserData(body)))
    };
}

// Drives a held entity's body toward the mouse via linear velocity, so Box2D
// integrates the motion and resolves collisions for the dragged object. The
// resulting position flows back into Transform via syncTransformFromBody.
void holdFollow(bagel::Entity e, const DragIntent& intent)
{
    const WorldPos target =
        screenToWorldPoint(intent.mousePos, RenderContext::getCameraPos());

    if (!e.has<PhysicsBody>()) return;

    const b2BodyId body = e.get<PhysicsBody>().id;
    if (!b2Body_IsValid(body)) return;

    // No gravity while held, so a body resting in the deadzone does not fall.
    b2Body_SetGravityScale(body, 0.f);

    const b2Vec2 current = b2Body_GetPosition(body);
    const b2Vec2 delta   = b2Sub({ target.x, target.y }, current);

    if (b2LengthSquared(delta) <= ARRIVE_THRESHOLD_SQ)
    {
        b2Body_SetLinearVelocity(body, { 0.f, 0.f });
        return;
    }

    // Clamp speed so one frame cannot carry the body past the target. A constant
    // MAX_FOLLOW_SPEED overshoots when closer than one step of travel, which makes a
    // stationary held body oscillate between the two sides of the cursor.
    const float  dist  = b2Length(delta);
    const float  speed = speedByDist(dist);
    const b2Vec2 dir   = b2Normalize(delta);
    b2Body_SetLinearVelocity(body, b2MulSV(speed, dir));
}

// Snaps a released entity onto its drop space (or restores gravity) and resets
// the intent back to None. Sensors stay permanently enabled; drop-space gating
// is done purely in software by reading DragIntent state, so there is no sensor
// toggling here.
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
                b2Body_SetLinearVelocity(body, { 0.f, 0.f });
                b2Body_SetGravityScale(body, 0.f);
            }
        }
    }
    else if (e.has<PhysicsBody>())
    {
        const b2BodyId body = e.get<PhysicsBody>().id;
        if (b2Body_IsValid(body))
            b2Body_SetGravityScale(body, 1.f);
    }

    intent.intentType      = DragIntentType::None;
    intent.dropSpaceEntity = std::nullopt;
}

} // namespace

void addDraggableVisitorShape(b2BodyId body, float halfW, float halfH)
{
    b2ShapeDef visitor = b2DefaultShapeDef();
    visitor.isSensor            = true;
    // Assign, don't OR: the b2DefaultShapeDef filter is { categoryBits = 1,
    // maskBits = UINT64_MAX }, and category bit 1 == filter::LIQUID. ORing would
    // leave the shape tagged LIQUID and colliding with every category.
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

    // Single pass: branch per entity on its drag state. Field mutation only
    // (no add/del), so iterating while mutating is safe.
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask)) continue;

        auto& intent = e.get<DragIntent>();
        switch (intent.intentType)
        {
        case DragIntentType::held:
            holdFollow(e, intent);
            break;

        case DragIntentType::released:
            releaseEntity(e, intent);
            break;

        case DragIntentType::None:
            break;
        }
    }
}

void dropSpaceDetectionSystem(PhysicsContext& physics)
{
    static const bagel::Mask heldMask =
        bagel::MaskBuilder().set<DragIntent>().set<DragItemType>().build();
    static const bagel::Mask dropSpaceMask = bagel::MaskBuilder().set<DropSpace>().build();

    // Accumulated across every physics sub-step this frame (Box2D clears its own
    // event buffer each b2World_Step).
    for (const auto& be : physics.sensorBeginEvents())
    {
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
        // DropType::Any accepts any draggable; otherwise the types must match.
        const DropType accepts = sensor.get<DropSpace>().dropType;
        const DropType carried = visitor.get<DragItemType>().dropType;
        if (accepts == DropType::Any || accepts == carried)
            intent.dropSpaceEntity = sensor.entity();
    }

    for (const auto& ee : physics.sensorEndEvents())
    {
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
