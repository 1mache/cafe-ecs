#include "Systems.h"
#include "Components.h"
#include "RenderContext.h"
#include "TransformOperations.h"
#include <bagel.h>

namespace cafe
{
void drawSystem()
{
    static const bagel::Mask mask =
        bagel::MaskBuilder().set<Drawable>().set<Transform>().build();

    SDL_Window*   window   = RenderContext::getWindow();
    SDL_Renderer* renderer = RenderContext::getRenderer();
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask))
            continue;

        const auto& d = e.get<Drawable>();
        const auto& t = e.get<Transform>();

        SDL_FRect dstRect = transformToFrect(t, RenderContext::getCameraPos());

        SDL_RenderTexture(renderer, d.texture, &d.srcRect, &dstRect);
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