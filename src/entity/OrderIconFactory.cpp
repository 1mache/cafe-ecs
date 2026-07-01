#include "OrderIconFactory.h"
#include "AssetManager.h"
#include "Components.h"
#include "RenderLayers.h"
#include "SpriteDims.h"
#include "SpriteSheet.h"
#include "Texture.h"
#include <bagel.h>

namespace cafe
{

namespace
{
static constexpr auto TEX         = "props.png";
static constexpr auto SPRITE_DATA = "props.json";
}

bagel::Entity createOrderIcon(AssetManager& assets, int propId,
                              float displayW, float displayH,
                              bagel::Entity parentBubble, SDL_FPoint offsetPx)
{
    const Texture&     tex   = assets.getTexture(TEX);
    const SpriteSheet& props = assets.getSpriteSheet(TEX, SPRITE_DATA);
    SDL_FRect srcRect = props.getFrameRect(propId);
    auto ent = bagel::Entity::create();
    ent.addAll(
        Transform{.w = texToWorldScale(displayW), .h = texToWorldScale(displayH)},
        Drawable{tex.get(), srcRect, layer::UI2},
        ChildOf{parentBubble, offsetPx});
    return ent;
}

} // namespace cafe
