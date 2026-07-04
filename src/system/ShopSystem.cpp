#include "ShopSystem.h"

#include "AudioContext.h"
#include "Components.h"      // ShopButton, NextDayButton, Transform
#include "IntentSystem.h"    // mouseWindowToRenderPoint
#include "RenderContext.h"
#include "SoundAssets.h"
#include "Transform.h"       // isPointInsideTransform, screenToWorldPoint
#include "UpgradeState.h"
#include <bagel.h>

namespace cafe
{
void shopInputSystem(SDL_Renderer* renderer, bool& outNextDay, bool& outExit, AudioContext& audio)
{
    static const bagel::Mask shopMask =
        bagel::MaskBuilder().set<ShopButton>().set<Transform>().build();
    static const bagel::Mask nextMask =
        bagel::MaskBuilder().set<NextDayButton>().set<Transform>().build();

    bool       clicked = false;
    SDL_FPoint clickPos{};

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            outExit = true;
            return;
        case SDL_EVENT_KEY_DOWN:
            if (event.key.scancode == SDL_SCANCODE_RETURN ||
                event.key.scancode == SDL_SCANCODE_SPACE)
                outNextDay = true;
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            clicked  = true;
            clickPos = mouseWindowToRenderPoint(renderer, event.button.x, event.button.y);
            break;
        default:
            break;
        }
    }

    if (!clicked) return;

    const WorldPos worldMouse = screenToWorldPoint(clickPos, RenderContext::getCameraPos());

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (e.test(shopMask) && isPointInsideTransform(worldMouse, e.get<Transform>()))
        {
            e.get<ShopButton>().justPressed = true;
            audio.play(sound::BUTTON_PRESS, sound::BUTTON_VOLUME);
        }
        else if (e.test(nextMask) && isPointInsideTransform(worldMouse, e.get<Transform>()))
        {
            outNextDay = true;
            audio.play(sound::BUTTON_PRESS, sound::BUTTON_VOLUME);
        }
    }
}

void shopPurchaseSystem(AudioContext& audio)
{
    static const bagel::Mask shopMask = bagel::MaskBuilder().set<ShopButton>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(shopMask)) continue;
        auto& b = e.get<ShopButton>();
        if (!b.justPressed) continue;
        b.justPressed = false;
        if (UpgradeState::tryBuy(b.id))   // no-op + false if unaffordable or maxed
            audio.play(sound::BUY_UPGRADE);
    }
}
} // namespace cafe
