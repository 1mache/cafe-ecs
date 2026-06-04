#include "Entities.h"
#include "GameConfig.h"
#include <cassert>

namespace cafe
{

bagel::Entity createClient(SDL_Texture* tex, float texW, float texH,
                           WorldPos pos, const Order& order, float patience)
{
    assert((order.hasDrink || order.hasPastry) && "createClient: order must have at least one item");

    auto ent = bagel::Entity::create();
    ent.addAll(
        Transform{.x = pos.x, .y = pos.y, .w = texW / (2.f * PTM), .h = texH / (2.f * PTM)},
        Drawable{tex, {0.f, 0.f, texW, texH}},
        order,
        Behavior{.patience = patience});
    return ent;
}

bagel::Entity createSpeechBubble(SDL_Texture* tex, float texW, float texH,
                                 bagel::Entity parent, SDL_FPoint offsetPx)
{
    auto ent = bagel::Entity::create();
    ent.addAll(
        Transform{.w = texW / (2.f * PTM), .h = texH / (2.f * PTM)},
        Drawable{tex, {0.f, 0.f, texW, texH}, /*renderLayer*/ 10},
        ChildOf{parent, offsetPx});
    return ent;
}

bagel::Entity createOrderIcon(SDL_Texture* tex, SDL_FRect srcRect,
                              bagel::Entity parentBubble, SDL_FPoint offsetPx)
{
    auto ent = bagel::Entity::create();
    ent.addAll(
        Transform{.w = srcRect.w / (2.f * PTM), .h = srcRect.h / (2.f * PTM)},
        Drawable{tex, srcRect, /*renderLayer*/ 20},
        ChildOf{parentBubble, offsetPx});
    return ent;
}

} // namespace cafe