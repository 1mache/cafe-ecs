#include "Entities.h"
#include "Components.h"
#include <box2d/box2d.h>

namespace cafe
{
void destroyPhysicalEntity(bagel::ent_type id)
{
    bagel::Entity e{ id };
    if (e.has<PhysicsBody>())
        b2DestroyBody(e.get<PhysicsBody>().id);
    e.destroy();
}
} // namespace cafe
