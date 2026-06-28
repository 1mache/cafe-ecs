#include "TransformSystem.h"
#include "Components.h"
#include "Transform.h"
#include <bagel.h>
#include <box2d/box2d.h>

namespace cafe
{
void physicsToTransformSystem()
{
    static const bagel::Mask mask =
        bagel::MaskBuilder().set<Transform>().set<PhysicsBody>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask)) continue;
        const auto body = e.get<PhysicsBody>().id;
        if (!b2Body_IsValid(body)) continue;
        b2Vec2 p   = b2Body_GetPosition(body);
        b2Rot  rot = b2Body_GetRotation(body);
        auto&  t   = e.get<Transform>();
        t.x   = p.x;
        t.y   = p.y;
        t.rot = b2Rot_GetAngle(rot);
    }
}

void hierarchySystem()
{
    static const bagel::Mask childMask =
        bagel::MaskBuilder().set<ChildOf>().set<Transform>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(childMask)) continue;

        auto& childComp = e.get<ChildOf>();
        bagel::Entity parent = childComp.parent;

        // If the parent is gone or leaving, destroy this child immediately
        if (!parent.has<Transform>() || parent.has<Leaving>())
        {
            e.destroy();
            continue;
        }

        auto& t       = e.get<Transform>();
        auto& parentT = parent.get<Transform>();

        float worldOffsetX = childComp.isWorldOffset
                                 ? childComp.localOffset.x
                                 : texToWorldDistance(childComp.localOffset.x);
        float worldOffsetY = childComp.isWorldOffset
                                 ? childComp.localOffset.y
                                 : texToWorldDistance(childComp.localOffset.y);

        t.x   = parentT.x + worldOffsetX;
        t.y   = parentT.y + worldOffsetY;
        t.rot = parentT.rot;
    }
}

} // namespace cafe
