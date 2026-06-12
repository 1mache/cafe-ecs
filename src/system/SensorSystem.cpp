#include "SensorSystem.h"
#include "Components.h"
#include <algorithm>
#include <bagel.h>
#include <box2d/box2d.h>
#include <cstdint>
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

} // namespace cafe
