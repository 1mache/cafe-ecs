#include "DestroySystem.h"

#include "Components.h"
#include "Entities.h"

#include <SDL3/SDL.h>
#include <bagel.h>
#include <vector>

namespace cafe
{
namespace
{
// True if id refers to a live, Destroy-tagged entity. Guards the -1 default
// holdingContainer (liquid/ice not yet caught by any cup) before touching it.
bool ownerTaggedDestroy(bagel::ent_type owner)
{
    if (owner.id < 0) return false;
    return bagel::Entity{ owner }.has<Destroy>();
}
} // namespace

void destroySystem()
{
    static const bagel::Mask childMask   = bagel::MaskBuilder().set<ChildOf>().build();
    static const bagel::Mask liquidMask  = bagel::MaskBuilder().set<Liquid>().build();
    static const bagel::Mask iceMask     = bagel::MaskBuilder().set<Ice>().build();
    static const bagel::Mask cupMask     = bagel::MaskBuilder().set<Cup>().build();
    static const bagel::Mask pastryMask  = bagel::MaskBuilder().set<Pastry>().build();
    static const bagel::Mask destroyMask = bagel::MaskBuilder().set<Destroy>().build();

    // Seeds: entities other systems already tagged Destroy this frame.
    std::vector<bagel::ent_type> toDestroy;
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
        if (e.test(destroyMask))
            toDestroy.push_back(e.entity());

    // Closure: propagate Destroy to anything referencing an already-tagged
    // entity (ChildOf.parent, Liquid/Ice.holdingContainer), collecting each
    // newly-tagged id the moment it's tagged. Looped to a fixpoint so
    // multi-level chains resolve fully. The `continue` guard means an entity
    // flips untagged -> tagged exactly once, so pushing inline here can't
    // duplicate or miss anything discovered on a later pass. Nothing is
    // destroyed during this loop, so no id is freed/reused mid-pass.
    bool changed = true;
    while (changed)
    {
        changed = false;
        for (auto e = bagel::Entity::first(); !e.eof(); e.next())
        {
            if (e.test(destroyMask)) continue;

            bool owned = false;
            if (e.test(childMask))
                owned = ownerTaggedDestroy(e.get<ChildOf>().parent.entity());
            if (!owned && e.test(liquidMask))
                owned = ownerTaggedDestroy(e.get<Liquid>().holdingContainer);
            if (!owned && e.test(iceMask))
                owned = ownerTaggedDestroy(e.get<Ice>().holdingContainer);

            if (owned)
            {
                e.addAll(Destroy{});
                toDestroy.push_back(e.entity());
                changed = true;
            }
        }
    }

    // Destroy. destroyPhysicalEntity is idempotent, and each id here is
    // unique (the tag makes membership set-like), so no dedupe needed.
    for (auto id : toDestroy)
    {
        bagel::Entity e{ id };
        if (e.test(iceMask))
            SDL_Log("destroySystem: destroyed entity %d (Ice)", id.id);
        else if (e.test(liquidMask))
            SDL_Log("destroySystem: destroyed entity %d (Liquid, kind=%d)",
                     id.id, static_cast<int>(e.get<Liquid>().kind));
        else if (e.test(cupMask))
            SDL_Log("destroySystem: destroyed entity %d (Cup)", id.id);
        else if (e.test(pastryMask))
            SDL_Log("destroySystem: destroyed entity %d (Pastry, type=%d)",
                     id.id, static_cast<int>(e.get<Pastry>().type));

        destroyPhysicalEntity(id);
    }
}
} // namespace cafe
