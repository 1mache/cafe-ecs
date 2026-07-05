#include "NapkinFactory.h"

#include "AssetManager.h"
#include "Components.h"
#include "RenderLayers.h"
#include "Texture.h"

namespace cafe
{
bagel::Entity createNapkin(AssetManager& assets)
{
    static constexpr auto TEX = "napkin.png";
    const Texture& tex = assets.getTexture(TEX);
    const SDL_FRect src = tex.getFullSrcRect();

    auto ent = bagel::Entity::create();

    ent.addAll(
        Transform{ .x = NAPKIN_HIDDEN_CENTER_X,
                   .y = NAPKIN_HIDDEN_CENTER_Y,
                   .w = NAPKIN_HIDDEN_HALF_W,
                   .h = NAPKIN_HIDDEN_HALF_H },
        Drawable{ tex.get(), src, layer::NAPKIN },
        NapkinIntent{});
    return ent;
}
} // namespace cafe
