#include "SpeechBubbleFactory.h"
#include "AssetManager.h"
#include "Components.h"
#include "GameConfig.h"
#include "Texture.h"
#include <bagel.h>

namespace cafe
{

namespace
{
static constexpr auto TEX = "bubble.png";
}

bagel::Entity createSpeechBubble(AssetManager& assets,
                                 float displayW, float displayH,
                                 bagel::Entity parent, SDL_FPoint offsetPx)
{
    const Texture& tex = assets.getTexture(TEX);
    auto ent = bagel::Entity::create();
    ent.addAll(
        Transform{.w = screenToWorldScale(displayW), .h = screenToWorldScale(displayH)},
        Drawable{tex.get(), tex.getFullSrcRect(), /*renderLayer*/ 10},
        ChildOf{parent, offsetPx});
    return ent;
}

} // namespace cafe
