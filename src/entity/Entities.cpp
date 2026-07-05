#include "Entities.h"
#include "Components.h"
#include <box2d/box2d.h>
#include <cmath>
#include <vector>

namespace cafe
{
void destroyPhysicalEntity(bagel::ent_type ent)
{
    bagel::Entity e{ ent };
    // Empty mask = already destroyed. bagel's deleteEntity pushes the id onto
    // its free-list unconditionally, so a second destroy would hand the same id
    // to two live entities later (they'd share mask + components, and deleting
    // one deletes both). Skipping here makes every destroy path idempotent.
    if (e.mask().ctz() < 0)
        return;
    if (e.has<PhysicsBody>())
        b2DestroyBody(e.get<PhysicsBody>().id);
    e.destroy();
}

void destroyAllGameEntities()
{
    std::vector<bagel::ent_type> toDestroy;
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
        toDestroy.push_back(e.entity());

    for (auto id : toDestroy)
        destroyPhysicalEntity(id);
}

void rejectItem(bagel::Entity e)
{
    if (!e.has<PhysicsBody>()) return;
    const b2BodyId body = e.get<PhysicsBody>().id;
    if (!b2Body_IsValid(body)) return;

    constexpr float REJECT_IMPULSE = 300.f;
    b2Body_SetGravityScale(body, 1.f);
    b2Vec2 incomingVelocity = b2Body_GetLinearVelocity(body);
    b2Body_SetLinearVelocity(body, {});
    const float rejectAngle = std::atan2(-incomingVelocity.y, -incomingVelocity.x);
    b2Body_ApplyLinearImpulseToCenter(
        body, { REJECT_IMPULSE * std::cos(rejectAngle), REJECT_IMPULSE * std::sin(rejectAngle) }, true);
}
} // namespace cafe
