#include "Entities.h"
#include "GameConfig.h"
#include <cassert>

namespace cafe
{

bagel::Entity createClient(SDL_Texture* tex, float texW, float texH,
                           WorldPos pos, const Order& order, float patience,
                           SDL_FPoint mouthOffsetPx)
{
    assert((order.hasDrink || order.hasPastry) && "createClient: order must have at least one item");

    auto ent = bagel::Entity::create();
    ent.addAll(
        Transform{.x = pos.x, .y = pos.y, .w = texW / (2.f * PTM), .h = texH / (2.f * PTM)},
        Drawable{tex, {0.f, 0.f, texW, texH}},
        order,
        Behavior{.patience = patience},
        SpeechAnchor{.mouthOffset = mouthOffsetPx});
    return ent;
}

bagel::Entity createSpeechBubble(SDL_Texture* tex, SDL_FRect srcRect,
                                 float displayW, float displayH,
                                 bagel::Entity parent, SDL_FPoint offsetPx)
{
    auto ent = bagel::Entity::create();
    ent.addAll(
        Transform{.w = displayW / (2.f * PTM), .h = displayH / (2.f * PTM)},
        Drawable{tex, srcRect, /*renderLayer*/ 10},
        ChildOf{parent, offsetPx});
    return ent;
}

bagel::Entity createOrderIcon(SDL_Texture* tex, SDL_FRect srcRect,
                              float displayW, float displayH,
                              bagel::Entity parentBubble, SDL_FPoint offsetPx)
{
    auto ent = bagel::Entity::create();
    ent.addAll(
        Transform{.w = displayW / (2.f * PTM), .h = displayH / (2.f * PTM)},
        Drawable{tex, srcRect, /*renderLayer*/ 20},
        ChildOf{parentBubble, offsetPx});
    return ent;
}

} // namespace cafe