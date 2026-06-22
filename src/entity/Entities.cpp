#include "Entities.h"
#include "Components.h"
#include <box2d/box2d.h>
#include <vector>

namespace cafe
{
void destroyPhysicalEntity(bagel::ent_type id)
{
    bagel::Entity e{ id };
    if (e.has<PhysicsBody>())
        b2DestroyBody(e.get<PhysicsBody>().id);
    e.destroy();
}

void destroyDeliveredItem(bagel::ent_type id)
{
    bagel::Entity item{ id };
    if (!item.has<Cup>())
    {
        destroyPhysicalEntity(id);
        return;
    }

    // Cup: collect drops, ice, and child sprites that belong to this cup.
    static const bagel::Mask liquidMask = bagel::MaskBuilder().set<Liquid>().build();
    static const bagel::Mask iceMask    = bagel::MaskBuilder().set<Ice>().build();
    static const bagel::Mask childMask  = bagel::MaskBuilder().set<ChildOf>().build();

    std::vector<bagel::ent_type> toDestroy;
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (e.test(liquidMask) && e.get<Liquid>().holdingContainer.id == id.id)
            toDestroy.push_back(e.entity());
        else if (e.test(iceMask) && e.get<Ice>().holdingContainer.id == id.id)
            toDestroy.push_back(e.entity());
        else if (e.test(childMask) && e.get<ChildOf>().parent.entity().id == id.id)
            toDestroy.push_back(e.entity());
    }

    for (auto dep : toDestroy)
        destroyPhysicalEntity(dep);
    destroyPhysicalEntity(id);
}
} // namespace cafe
