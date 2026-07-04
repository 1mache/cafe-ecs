#include "TweenSystem.h"

#include "Transform.h"
#include "Tween.h"

namespace cafe
{
void tweenSystem(float dt)
{
    static auto mask = bagel::MaskBuilder().set<Transform>().set<Tween>().build();

    for (auto ent = bagel::Entity::first(); !ent.eof(); ent.next())
    {
        if (!ent.test(mask)) continue;

        auto& transform = ent.get<Transform>();
        auto& tween     = ent.get<Tween>();

        // if ended.
        if (tween.elapsedTime >= tween.duration)
            ent.del<Tween>(); // remove the component

        // else ease into target transform
        tweenTick(tween, dt, transform);

    }
};
} // namespace cafe