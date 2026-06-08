#include "CafeEnvironmentFactory.h"

#include "Components.h"

namespace cafe
{

bagel::Entity createBg(const Texture& bgTex)
{
    const auto bgSrcRect = bgTex.getFullSrcRect();

    auto bgEnt = bagel::Entity::create();
    bgEnt.addAll(Drawable{bgTex.get(), bgSrcRect},
                 Transform{.x = 0.f,
                           .y = 0.f,
                           .w = screenToWorldSize(LOGICAL_W/2),
                           .h = screenToWorldSize(LOGICAL_H/2)});
    return bgEnt;
}
bagel::Entity createBartop(const Texture& bartopTex)
{
    const auto  bartopSrcRect    = bartopTex.getFullSrcRect();
    const float bartopHalfHeight = screenToWorldSize(bartopSrcRect.h / 2.f);
    auto  bartopEnt        = bagel::Entity::create();
    bartopEnt.addAll(
        Drawable{bartopTex.get(), bartopSrcRect},
        Transform{.x = 0.f,
                  .y = -(screenToWorldSize(LOGICAL_H / 2) - bartopHalfHeight),
                  .w = screenToWorldSize(bartopSrcRect.w / 2.f),
                  .h = bartopHalfHeight});

    return bartopEnt;
}
} // namespace cafe