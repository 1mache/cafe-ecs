#include "Systems.h"
#include "Components.h"
#include "TransformOperations.h"
#include "RenderContext.h"
#include <bagel.h>

namespace cafe
{
void drawSystem()
{
    static const bagel::Mask mask =
        bagel::MaskBuilder().set<Drawable>().set<Transform>().build();

    SDL_Window* window = RenderContext::getWindow();
    SDL_Renderer* renderer = RenderContext::getRenderer();
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask)) continue;

        const auto& d = e.get<Drawable>();
        const auto& t = e.get<Transform>();

        SDL_FRect dstRect = transformToFrect(t, RenderContext::getCameraPos());

        for (const auto& sprite: d.sprites)
            SDL_RenderTexture(renderer, sprite.texture, &sprite.srcRect, &dstRect);

    }
}
} // namespace cafe