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
                           .w = screenToWorldScale(LOGICAL_W),
                           .h = screenToWorldScale(LOGICAL_H)});
    return bgEnt;
}
bagel::Entity createBartop(const Texture& bartopTex)
{
    const auto  bartopSrcRect    = bartopTex.getFullSrcRect();
    const float bartopHalfHeight = screenToWorldScale(bartopSrcRect.h);
    auto  bartopEnt        = bagel::Entity::create();
    bartopEnt.addAll(
        Drawable{bartopTex.get(), bartopSrcRect},
        Transform{.x = 0.f,
                  .y = -(screenToWorldDistance(LOGICAL_H / 2) - bartopHalfHeight),
                  .w = screenToWorldScale(bartopSrcRect.w),
                  .h = bartopHalfHeight});

    return bartopEnt;
}
} // namespace cafe