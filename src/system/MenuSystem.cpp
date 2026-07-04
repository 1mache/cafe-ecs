#include "MenuSystem.h"

#include "Components.h"      // StartButton, ExitButton, SoundToggleButton, Transform, Drawable
#include "IntentSystem.h"    // mouseWindowToRenderPoint
#include "RenderContext.h"
#include "SettingsState.h"
#include "Transform.h"       // isPointInsideTransform, screenToWorldPoint
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

void menuInputSystem(SDL_Renderer* renderer, bool& outStart, bool& outExit)
{
    static const bagel::Mask startMask =
        bagel::MaskBuilder().set<StartButton>().set<Transform>().build();
    static const bagel::Mask exitMask =
        bagel::MaskBuilder().set<ExitButton>().set<Transform>().build();
    static const bagel::Mask soundMask =
        bagel::MaskBuilder().set<SoundToggleButton>().set<Transform>().build();

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

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (e.test(startMask) && isPointInsideTransform(worldMouse, e.get<Transform>()))
            outStart = true;
        else if (e.test(exitMask) && isPointInsideTransform(worldMouse, e.get<Transform>()))
            outExit = true;
        else if (e.test(soundMask) && isPointInsideTransform(worldMouse, e.get<Transform>()))
            e.get<SoundToggleButton>().justPressed = true;
    }
}

void soundToggleSystem(AudioContext& audio)
{
    static const bagel::Mask soundMask =
        bagel::MaskBuilder().set<SoundToggleButton>().set<Drawable>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(soundMask)) continue;

        auto& b = e.get<SoundToggleButton>();
        if (b.justPressed)
        {
            b.justPressed = false;
            SettingsState::toggleMuted();
            audio.setVolume(SettingsState::muted() ? 0.f : 1.f);
        }
        // State-driven tint every frame: also paints the correct colour on the
        // very first update, so onInit doesn't need to know the colours.
        e.get<Drawable>().tint = SettingsState::muted() ? COLOR_MUTED : COLOR_SOUND_ON;
    }
}
} // namespace cafe
