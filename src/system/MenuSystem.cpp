#include "MenuSystem.h"

#include "ButtonSystem.h"     // updateButtonsFromMouse
#include "IntentSystem.h"    // mouseWindowToRenderPoint
#include "RenderContext.h"
#include "Transform.h"       // screenToWorldPoint
#include <bagel.h>

namespace cafe
{
void menuInputSystem(SDL_Renderer* renderer, bool& outExit, AudioContext& audio)
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
