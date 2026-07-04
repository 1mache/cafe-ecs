#include "ParticleSystem.h"

#include "Components.h"
#include "Entities.h" // destroyPhysicalEntity
#include "Transform.h"

#include <bagel.h>
#include <vector>

namespace cafe
{
void particleSystem(float dt)
{
    static const bagel::Mask mask =
        bagel::MaskBuilder().set<Particle>().set<Transform>().set<Lifetime>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask)) continue;

        const Particle& p  = e.get<Particle>();
        const Lifetime& lt = e.get<Lifetime>();
        auto&           t  = e.get<Transform>();

        t.x += p.velocity.x * dt;
        t.y += p.velocity.y * dt;

        // Linear fade 255 -> 0 over the lifetime; clamp in case age overshoots.
        float remain = 1.f - (lt.duration > 0.f ? lt.age / lt.duration : 1.f);
        if (remain < 0.f) remain = 0.f;
        const auto alpha = static_cast<Uint8>(255.f * remain);

        if (e.has<Drawable>())  e.get<Drawable>().tint.a  = alpha;
        if (e.has<TextLabel>()) e.get<TextLabel>().tint.a = alpha;
    }
}

void lifetimeSystem(float dt)
{
    static const bagel::Mask mask =
        bagel::MaskBuilder().set<Lifetime>().build();

    std::vector<bagel::ent_type> expired;
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask)) continue;
        auto& lt = e.get<Lifetime>();
        lt.age += dt;
        if (lt.age >= lt.duration)
            expired.push_back(e.entity());
    }

    for (auto id : expired)
        destroyPhysicalEntity(id);
}
} // namespace cafe
