#include "OrderIconFactory.h"
#include "AssetManager.h"
#include "Components.h"
#include "GameConfig.h"
#include "SpriteDims.h"
#include "Texture.h"
#include <bagel.h>

namespace cafe
{

namespace
{
static constexpr auto TEX = "props.png";
}

bagel::Entity createOrderIcon(AssetManager& assets, int propId,
                              float displayW, float displayH,
                              bagel::Entity parentBubble, SDL_FPoint offsetPx)
{
    const Texture& tex = assets.getTexture(TEX);
    float propIdF = static_cast<float>(propId);
    SDL_FRect srcRect{PROP_DIMS.x * propIdF, 0, PROP_DIMS.x, PROP_DIMS.y};
    auto ent = bagel::Entity::create();
    ent.addAll(
        Transform{.w = screenToWorldScale(displayW), .h = screenToWorldScale(displayH)},
        Drawable{tex.get(), srcRect, LAYER_UI2},
        ChildOf{parentBubble, offsetPx});
    return ent;
}

} // namespace cafe
