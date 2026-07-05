#include "ShopSystem.h"

#include "AudioContext.h"
#include "ButtonSystem.h"     // updateButtonsFromMouse
#include "IntentSystem.h"    // mouseWindowToRenderPoint
#include "RenderContext.h"
#include "Transform.h"       // screenToWorldPoint
#include <bagel.h>

namespace cafe
{
void shopInputSystem(SDL_Renderer* renderer, bool& outNextDay, bool& outExit, AudioContext& audio)
{
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
    updateButtonsFromMouse(worldMouse, clicked, /*mouseUp=*/false, audio);
}
} // namespace cafe
