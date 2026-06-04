#include "Systems.h"
#include "Components.h"
#include "RenderContext.h"
#include "TransformOperations.h"
#include <bagel.h>
#include <iostream>

namespace cafe
{

void drawSystem()
{
    static const bagel::Mask drawMask =
        bagel::MaskBuilder().set<Drawable>().set<Transform>().build();

    SDL_Renderer* renderer = RenderContext::getRenderer();
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(drawMask)) continue;

        const auto& d = e.get<Drawable>();
        const auto& t = e.get<Transform>();

        SDL_FRect dstRect = transformToFrect(t, RenderContext::getCameraPos());
        SDL_RenderTexture(renderer, d.texture, &d.srcRect, &dstRect);
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
        t.x   = parentT.x + screenToWorldSize(childComp.localOffset.x);
        t.y   = parentT.y + screenToWorldSize(childComp.localOffset.y);
        t.rot = parentT.rot;
    }
}

void behaviorSystem(float dtSeconds)
{
    static const bagel::Mask behaviorMask =
        bagel::MaskBuilder().set<Behavior>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(behaviorMask)) continue;

        auto& behavior = e.get<Behavior>();
        behavior.patience -= dtSeconds;

        // DEBUG: print patience once per second
        static float printAccum = 0.f;
        printAccum += dtSeconds;
        if (printAccum >= 1.f)
        {
            std::cout << "[behaviorSystem] entity " << e.entity().id
                      << " patience: " << behavior.patience << "s\n";
            printAccum = 0.f;
        }

        if (behavior.patience <= 0.f && !e.has<Leaving>())
            e.add(Leaving{});
    }
}

void orderSystem()
{
    static const bagel::Mask orderMask =
        bagel::MaskBuilder().set<Order>().set<Behavior>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(orderMask)) continue;

        // TODO: when a cup is served to this client, compare cup's Holds composition
        //       against Order::ratio and write the result into Behavior::rating
    }
}

void cleanupSystem()
{
    static const bagel::Mask leavingMask =
        bagel::MaskBuilder().set<Leaving>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(leavingMask)) continue;
        e.destroy();
    }
}

} // namespace cafe
