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
                                 bagel::Entity parent, SDL_FPoint offsetPx)
{
    const Texture& tex = assets.getTexture(TEX);
    auto [w, h] = tex.getSize();
    auto ent = bagel::Entity::create();
    ent.addAll(
        Transform{.w = screenToWorldScale(static_cast<float>(w)),
                  .h = screenToWorldScale(static_cast<float>(h))},
        Drawable{tex.get(), tex.getFullSrcRect(), LAYER_UI1},
        ChildOf{parent, offsetPx});
    return ent;
}

} // namespace cafe
