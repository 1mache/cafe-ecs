#include "NapkinFactory.h"

#include "AssetManager.h"
#include "Components.h"
#include "GameConfig.h"
#include "RenderLayers.h"
#include "Texture.h"

namespace cafe
{
namespace
{
constexpr float NAPKIN_SCALE = 0.25f;
} // namespace

bagel::Entity createNapkin(AssetManager& assets)
{
    static constexpr auto TEX = "napkin.png";
    const Texture& tex = assets.getTexture(TEX);
    const SDL_FRect src = tex.getFullSrcRect();

    const float halfW = texToWorldScale(src.w) * NAPKIN_SCALE;
    const float halfH = texToWorldScale(src.h) * NAPKIN_SCALE;
    const float bottomY = -texToWorldDistance(LOGICAL_H / 2);

    auto ent = bagel::Entity::create();
    ent.addAll(
        Transform{ .x = 0.f, .y = bottomY, .w = halfW, .h = halfH },
        Drawable{ tex.get(), src, layer::UI1 });
    return ent;
}
} // namespace cafe
