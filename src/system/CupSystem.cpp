#include "CupSystem.h"
#include "Components.h"
#include "PhysicsFilters.h"
#include <SDL3/SDL.h>
#include <bagel.h>
#include <box2d/box2d.h>

namespace cafe
{
namespace
{
// Poll the cup's interior sensor for any current overlap. Polling (not sensor
// events) because the fixed-timestep loop runs multiple b2World_Steps per frame
// and each step clears the begin-event buffer, so begin events get eaten before
// any system reads them. Current-overlap polling is immune to that.
bool cupHasContents(b2BodyId cupBody)
{
    if (!b2Body_IsValid(cupBody)) return false;

    b2ShapeId shapes[16];
    const int count = b2Body_GetShapes(cupBody, shapes, 16);
    for (int i = 0; i < count; ++i)
    {
        if (!b2Shape_IsSensor(shapes[i])) continue;
        if (!(b2Shape_GetFilter(shapes[i]).categoryBits & filter::CUP_INSIDE)) continue;

        // Capacity == current overlap count. The interior sensor masks
        // LIQUID | ICE only, so any overlap is a drop or ice cube.
        if (b2Shape_GetSensorCapacity(shapes[i]) > 0) return true;
    }
    return false;
}
} // namespace

void cupAlphaSystem()
{
    // Fraction of full opacity the cup front drops to once it holds anything.
    static constexpr float ALPHA_FACTOR = 0.5f;

    static const bagel::Mask mask =
        bagel::MaskBuilder().set<ChildOf>().set<Drawable>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask)) continue;

        // only the cup front: a child of an entity carrying both Cup and a body
        bagel::Entity parent = e.get<ChildOf>().parent;
        if (!parent.has<Cup>() || !parent.has<PhysicsBody>()) continue;

        const bool hasContents = cupHasContents(parent.get<PhysicsBody>().id);

        auto& d = e.get<Drawable>();
        d.tint.a = hasContents
                       ? static_cast<Uint8>(ALPHA_FACTOR * 255)
                       : 255;
    }
}
} // namespace cafe
