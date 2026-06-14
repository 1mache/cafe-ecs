#include "SpeechBubbleFactory.h"
#include "AssetManager.h"
#include "Components.h"
#include "RenderLayers.h"
#include "SpriteDims.h"
#include "Texture.h"
#include <bagel.h>

namespace cafe
{

namespace
{
static constexpr auto  TEX   = "bubble.png";
static constexpr float SCALE = 1.f / 1.7f;
}

bagel::Entity createSpeechBubble(AssetManager& assets,
                                 bagel::Entity parent, SDL_FPoint offsetPx)
{
    const Texture& tex = assets.getTexture(TEX);
    auto ent = bagel::Entity::create();
    ent.addAll(
        Transform{.w = screenToWorldScale(BUBBLE_DIMS.x * SCALE),
                  .h = screenToWorldScale(BUBBLE_DIMS.y * SCALE)},
        Drawable{tex.get(), tex.getFullSrcRect(), layer::UI1},
        ChildOf{parent, offsetPx});
    return ent;
}

} // namespace cafe
