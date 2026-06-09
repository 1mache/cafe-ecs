#include "ClientFactory.h"
#include "AssetManager.h"
#include "Components.h"
#include "GameConfig.h"
#include "Texture.h"
#include <bagel.h>
#include <cassert>

namespace cafe
{

namespace
{
static constexpr auto TEX = "def_customer.png";
}

bagel::Entity createClient(AssetManager& assets,
                           WorldPos pos, const Order& order, float patience,
                           SDL_FPoint mouthOffsetPx)
{
    assert((order.hasDrink || order.hasPastry) && "createClient: order must have at least one item");

    const Texture& tex = assets.getTexture(TEX);
    auto [w, h] = tex.getSize();
    auto ent = bagel::Entity::create();
    ent.addAll(
        Transform{.x = pos.x, .y = pos.y, .w = screenToWorldScale(w), .h = screenToWorldScale(h)},
        Drawable{tex.get(), tex.getFullSrcRect()},
        order,
        Behavior{.patience = patience},
        SpeechAnchor{.mouthOffset = mouthOffsetPx});
    return ent;
}

} // namespace cafe
