#include "MenuSystem.h"

#include "ButtonSystem.h"     // updateButtonsFromMouse
#include "Components.h"      // Button, Drawable
#include "IntentSystem.h"    // mouseWindowToRenderPoint
#include "RenderContext.h"
#include "SettingsState.h"
#include "Transform.h"       // screenToWorldPoint
#include <bagel.h>

namespace cafe
{
namespace
{
// Sound-toggle square, tinted by state (white = on, gray = muted). Same
// colour-by-state idiom as DayReportScene's upgrade bars.
constexpr SDL_Color COLOR_SOUND_ON  = { 255, 255, 255, 255 };
constexpr SDL_Color COLOR_MUTED     = {  78,  78,  86, 255 };
} // namespace

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

void soundToggleSystem()
{
    static const bagel::Mask soundMask =
        bagel::MaskBuilder().set<Button>().set<Drawable>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(soundMask)) continue;
        if (e.get<Button>().kind != ButtonKind::Sound) continue;

        // State-driven tint every frame: also paints the correct colour on the
        // very first update, so onInit doesn't need to know the colours.
        e.get<Drawable>().tint = SettingsState::muted() ? COLOR_MUTED : COLOR_SOUND_ON;
    }
}
} // namespace cafe
