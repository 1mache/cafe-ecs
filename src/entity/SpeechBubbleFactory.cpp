#include "SpeechBubbleFactory.h"
#include "Components.h"
#include "GameConfig.h"
#include <bagel.h>

namespace cafe
{

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

} // namespace cafe
