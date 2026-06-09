#include "OrderIconFactory.h"
#include "AssetManager.h"
#include "Components.h"
#include "GameConfig.h"
#include "Texture.h"
#include <bagel.h>

namespace cafe
{

namespace
{
static constexpr auto TEX = "props.png";
}

bagel::Entity createOrderIcon(AssetManager& assets, SDL_FRect srcRect,
                              float displayW, float displayH,
                              bagel::Entity parentBubble, SDL_FPoint offsetPx)
{
    const Texture& tex = assets.getTexture(TEX);
    auto ent = bagel::Entity::create();
    ent.addAll(
        Transform{.w = screenToWorldScale(displayW), .h = screenToWorldScale(displayH)},
        Drawable{tex.get(), srcRect, /*renderLayer*/ 20},
        ChildOf{parentBubble, offsetPx});
    return ent;
}

} // namespace cafe
