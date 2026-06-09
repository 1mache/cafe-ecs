#include "CafeEnvironmentFactory.h"
#include "AssetManager.h"
#include "Components.h"
#include "Texture.h"

namespace cafe
{

namespace
{
static constexpr auto TEX_BARTOP = "counter.png";
}

bagel::Entity createBg(const Texture& bgTex)
{
    const auto bgSrcRect = bgTex.getFullSrcRect();

    auto bgEnt = bagel::Entity::create();
    bgEnt.addAll(Drawable{bgTex.get(), bgSrcRect, LAYER_BG},
                 Transform{.x = 0.f,
                           .y = 0.f,
                           .w = screenToWorldScale(LOGICAL_W),
                           .h = screenToWorldScale(LOGICAL_H)});
    return bgEnt;
}
bagel::Entity createBartop(AssetManager& assets)
{
    const Texture& bartopTex     = assets.getTexture(TEX_BARTOP);
    const auto  bartopSrcRect    = bartopTex.getFullSrcRect();
    const float bartopHalfHeight = screenToWorldScale(bartopSrcRect.h);
    auto  bartopEnt        = bagel::Entity::create();
    bartopEnt.addAll(
        Drawable{bartopTex.get(), bartopSrcRect, LAYER_BARTOP},
        Transform{.x = 0.f,
                  .y = -(screenToWorldDistance(LOGICAL_H / 2) - bartopHalfHeight),
                  .w = screenToWorldScale(bartopSrcRect.w),
                  .h = bartopHalfHeight});

    return bartopEnt;
}
} // namespace cafe