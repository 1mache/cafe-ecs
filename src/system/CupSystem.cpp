#include "CupSystem.h"
#include "Components.h"
#include <SDL3/SDL.h>
#include <bagel.h>
#include <iostream>

namespace cafe
{
void cupAlphaSystem()
{
    // Fraction of full opacity the cup front drops to once it holds anything.
    static constexpr float ALPHA_FACTOR = 0.5f;

    static const bagel::Mask mask =
        bagel::MaskBuilder().set<ChildOf>().set<Drawable>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask)) continue;

        // check that we are a child of a cup
        bagel::Entity parent = e.get<ChildOf>().parent;
        if (!parent.has<Cup>()) continue;

        auto&       d   = e.get<Drawable>();
        const auto& cup = parent.get<Cup>();

        const bool hasContents = cup.totalFilled() > 0 || cup.iceCount > 0;
        std::cout << "[CupAlpha] filled=" << cup.totalFilled()
                  << " ice=" << cup.iceCount
                  << " hasContents=" << hasContents << "\n";
        d.tint.a = hasContents
                       ? static_cast<Uint8>(ALPHA_FACTOR * 255)
                       : 255;
    }
}
} // namespace cafe
