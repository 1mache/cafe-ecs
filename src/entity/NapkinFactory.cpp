#include "NapkinFactory.h"

#include "AssetManager.h"
#include "Components.h"
#include "GameConfig.h"
#include "RenderLayers.h"
#include "Texture.h"

namespace cafe
{
// Must match res/napkin.png pixel size.
constexpr float NAPKIN_TEX_W = 100.f;
constexpr float NAPKIN_TEX_H = 90.f;
constexpr float NAPKIN_SCALE = 0.25f;

constexpr float HALF_W   = texToWorldScale(NAPKIN_TEX_W) * NAPKIN_SCALE;
constexpr float HALF_H   = texToWorldScale(NAPKIN_TEX_H) * NAPKIN_SCALE;
constexpr float CENTER_X = 0.f;
constexpr float CENTER_Y = -texToWorldDistance(LOGICAL_H / 2.f);

// Screen-space hitbox at creation time (camera at origin).
constexpr float SCREEN_X = LOGICAL_W * 0.5f + (CENTER_X - HALF_W) * PTM * SCALE_FACTOR;
constexpr float SCREEN_Y = LOGICAL_H * 0.5f - (CENTER_Y + HALF_H) * PTM * SCALE_FACTOR;
constexpr float SCREEN_W = worldToScreenScale(HALF_W);
constexpr float SCREEN_H = worldToScreenScale(HALF_H);

bagel::Entity createNapkin(AssetManager& assets)
{
    static constexpr auto TEX = "napkin.png";
    const Texture& tex = assets.getTexture(TEX);
    const SDL_FRect src = tex.getFullSrcRect();

    auto ent = bagel::Entity::create();
    ent.addAll(
        Transform{ .x = CENTER_X,
                   .y = CENTER_Y,
                   .w = HALF_W,
                   .h = HALF_H },
        Drawable{ tex.get(), src, layer::UI1 });
    return ent;
}
} // namespace cafe
