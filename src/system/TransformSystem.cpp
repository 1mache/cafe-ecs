#include "TransformSystem.h"
#include "Components.h"
#include "Transform.h"
#include <bagel.h>
#include <box2d/box2d.h>

namespace cafe
{
void syncTransformFromBody()
{
    static const bagel::Mask mask =
        bagel::MaskBuilder().set<Transform>().set<PhysicsBody>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask)) continue;
        const auto body = e.get<PhysicsBody>().id;
        if (!b2Body_IsValid(body)) continue;  // belt + suspenders: never sync a destroyed body
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
    static const bagel::Mask mask =
        bagel::MaskBuilder().set<Drawable>().set<Transform>().set<ChildOf>().build();
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask)) continue;

        auto& t = e.get<Transform>();
        auto& childComp = e.get<ChildOf>();
        auto& parentT = childComp.parent.get<Transform>();
        t.x = parentT.x + screenToWorldSize(childComp.localOffset.x);
        t.y = parentT.y + screenToWorldSize(childComp.localOffset.y);
        t.rot = parentT.rot;
    }
}
} // namespace cafe
