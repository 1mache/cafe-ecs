#include "SensorSystem.h"
#include "Components.h"
#include "PhysicsContext.h"
#include "PhysicsFilters.h"
#include "Utils.h"
#include <algorithm>
#include <bagel.h>
#include <box2d/box2d.h>
#include <cstdint>
#include <vector>

// Two sensor-querying mechanisms live in this file, both reading Box2D sensor
// state but through different APIs:
//   sensorGatheringSystem — polls b2Shape_GetSensorOverlaps every frame for
//     any Sensor-tagged body, writing InSensorArea onto whatever's currently
//     inside (drop-zone detection, dragAndDropSystem).
//   sensorEventSystem — drains the begin-touch event buffer PhysicsContext
//     accumulates (CUP_INSIDE fills, CLEANUP tags Destroy).
// Same domain, different query style; kept together rather than split by a
// name that would otherwise look identical from the outside.

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

bool sameEntity(bagel::ent_type lhs, bagel::ent_type rhs)
{
    return lhs.id == rhs.id;
}

bool entityLess(bagel::ent_type lhs, bagel::ent_type rhs)
{
    return lhs.id < rhs.id;
}

bool containsEntity(const std::vector<bagel::ent_type>& entities, bagel::ent_type ent)
{
    return std::binary_search(entities.begin(), entities.end(), ent, entityLess);
}

void gatherSensorShapeOverlaps(b2ShapeId shapeId,
                               bagel::ent_type sensorId,
                               std::vector<bagel::ent_type>& overlapping)
{
    if (!b2Shape_IsValid(shapeId)) return;

    const int capacity = b2Shape_GetSensorCapacity(shapeId);
    if (capacity <= 0) return;

    std::vector<b2ShapeId> overlaps(static_cast<size_t>(capacity));
    const int count = b2Shape_GetSensorOverlaps(shapeId, overlaps.data(), capacity);

    for (int i = 0; i < count; ++i)
    {
        const b2ShapeId visitorShape = overlaps[static_cast<size_t>(i)];
        if (!b2Shape_IsValid(visitorShape)) continue;

        const b2BodyId visitorBody = b2Shape_GetBody(visitorShape);
        if (!b2Body_IsValid(visitorBody)) continue;

        const bagel::ent_type visitorId = entityIdFromBody(visitorBody);
        if (sameEntity(visitorId, sensorId)) continue;

        bagel::Entity visitor{ visitorId };
        if (visitor.has<InSensorArea>())
            visitor.get<InSensorArea>().sensor = sensorId;
        else
            visitor.add(InSensorArea{ .sensor = sensorId });

        overlapping.push_back(visitorId);
    }
}
} // namespace

void sensorGatheringSystem()
{
    static const bagel::Mask sensorMask =
        bagel::MaskBuilder().set<Sensor>().set<PhysicsBody>().build();
    static const bagel::Mask inSensorAreaMask =
        bagel::MaskBuilder().set<InSensorArea>().build();

    std::vector<bagel::ent_type> overlapping;

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(sensorMask)) continue;

        const b2BodyId body = e.get<PhysicsBody>().id;
        if (!b2Body_IsValid(body)) continue;

        const int shapeCount = b2Body_GetShapeCount(body);
        if (shapeCount <= 0) continue;

        std::vector<b2ShapeId> shapes(static_cast<size_t>(shapeCount));
        b2Body_GetShapes(body, shapes.data(), shapeCount);

        for (b2ShapeId shapeId : shapes)
            gatherSensorShapeOverlaps(shapeId, e.entity(), overlapping);
    }

    std::sort(overlapping.begin(), overlapping.end(), entityLess);
    overlapping.erase(std::unique(overlapping.begin(), overlapping.end(), sameEntity),
                      overlapping.end());

    std::vector<bagel::ent_type> toRemove;
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(inSensorAreaMask)) continue;
        if (!containsEntity(overlapping, e.entity()))
            toRemove.push_back(e.entity());
    }

    for (bagel::ent_type ent : toRemove)
        bagel::Entity{ ent }.del<InSensorArea>();
}

void sensorEventSystem(PhysicsContext& physics)
{
    // Read the accumulated begin events: Box2D clears its own buffer each sub-step,
    // so PhysicsContext gathers every sub-step's events across the frame.
    const auto beginEvents = physics.sensorBeginEvents();

    static const bagel::Mask liquidMask = bagel::MaskBuilder().set<Liquid>().build();
    static const bagel::Mask cupMask    = bagel::MaskBuilder().set<Cup>().build();
    static const bagel::Mask iceMask    = bagel::MaskBuilder().set<Ice>().build();
    static const bagel::Mask pastryMask = bagel::MaskBuilder().set<Pastry>().build();

    for (const auto& be : beginEvents)
    {

        // Skip events whose shapes were destroyed since the buffer was filled.
        if (!b2Shape_IsValid(be.visitorShapeId)) continue;
        if (!b2Shape_IsValid(be.sensorShapeId))  continue;

        // Recover the visitor entity from the body's userData.
        const b2BodyId visitorBody = b2Shape_GetBody(be.visitorShapeId);
        const bagel::ent_type visitorId{
            static_cast<int>(reinterpret_cast<uintptr_t>(b2Body_GetUserData(visitorBody)))
        };
        bagel::Entity visitor{ visitorId };
        const bool visitorIsLiquid = visitor.test(liquidMask);
        const bool visitorIsIce    = visitor.test(iceMask);
        const bool visitorIsCup    = visitor.test(cupMask);
        const bool visitorIsPastry = visitor.test(pastryMask);
        if (!visitorIsLiquid && !visitorIsIce && !visitorIsCup && !visitorIsPastry) continue;

        // Which sensor was hit? Read the category bit on the sensor shape.
        const uint64_t sensorCat = b2Shape_GetFilter(be.sensorShapeId).categoryBits;

        if (sensorCat & filter::CUP_INSIDE)
        {
            const b2BodyId cupBody = b2Shape_GetBody(be.sensorShapeId);
            bagel::Entity cup{ bagel::ent_type{
                static_cast<int>(reinterpret_cast<uintptr_t>(b2Body_GetUserData(cupBody)))
            } };

            if (!cup.test(cupMask))
                fatalError("Object with filter CUP_INSIDE detected that is not cup");

            auto& c = cup.get<Cup>();
            if (visitorIsIce)
            {
                // Count an ice cube once, the first time it enters any cup.
                auto& ice = visitor.get<Ice>();
                if (ice.holdingContainer.id < 0)
                {
                    ++c.iceCount;
                    ice.holdingContainer = cup.entity(); // tag for per-cup cleanup on delivery
                }
            }
            else if (visitorIsLiquid)
            {
                ++c.filled[static_cast<size_t>(visitor.get<Liquid>().kind)];
                visitor.get<Liquid>().holdingContainer = cup.entity(); // tag for per-cup cleanup on delivery
            }
        }
        else if (sensorCat & filter::CLEANUP)
        {
            // A visitor can show up in more than one begin-event this frame (e.g.
            // brushing CLEANUP twice); Destroy is a tag, so re-tagging is a
            // no-op. destroySystem (end of frame) does the actual destroy + logs it.
            visitor.addAll(Destroy{});
        }
    }
}
} // namespace cafe
