#include "IntentSystem.h"
#include "AudioContext.h"
#include "ButtonSystem.h"   // updateButtonsFromMouse
#include "Components.h"
#include "RenderContext.h"
#include "SoundAssets.h"
#include "Transform.h"
#include "UpgradeState.h"
#include "UserInput.h"

#include <bagel.h>
#include <iostream>

namespace cafe
{
namespace
{
bool isPointInsideScreenRect(SDL_FPoint p, float x, float y, float w, float h)
{
    return p.x >= x && p.x <= x + w &&
           p.y >= y && p.y <= y + h;
}

void updateNapkinIntent(bagel::Entity e, const UserInput& input)
{
    auto& intent = e.get<NapkinIntent>();

    if (intent.state == NapkinState::Hidden)
    {
        if (isPointInsideScreenRect(input.mousePos,
                                    NAPKIN_HIDDEN_HITBOX_X,
                                    NAPKIN_HIDDEN_HITBOX_Y,
                                    NAPKIN_HIDDEN_HITBOX_W,
                                    NAPKIN_HIDDEN_HITBOX_H))
        {
            intent.state = NapkinState::Toggle;
            std::cout << "NapkinIntent: switched to Toggle (hover over hidden hitbox)\n";
        }
    }
    else if (intent.state == NapkinState::Toggle)
    {
        if (isPointInsideScreenRect(input.mousePos,
                                    NAPKIN_TOGGLE_HITBOX_X,
                                    NAPKIN_TOGGLE_HITBOX_Y,
                                    NAPKIN_TOGGLE_HITBOX_W,
                                    NAPKIN_TOGGLE_HITBOX_H) &&
            (input.controls & controlBit(Controls::MouseButtonDown)))
        {
            intent.state = NapkinState::FullBlank;
            std::cout << "NapkinIntent: switched to FullBlank (click inside toggle hitbox)\n";
        }
        else if (!isPointInsideScreenRect(input.mousePos,
                                          NAPKIN_TOGGLE_HITBOX_X,
                                          NAPKIN_TOGGLE_HITBOX_Y,
                                          NAPKIN_TOGGLE_HITBOX_W,
                                          NAPKIN_TOGGLE_HITBOX_H))
        {
            intent.state = NapkinState::Hidden;
            std::cout << "NapkinIntent: switched to Hidden (mouse left toggle hitbox)\n";
        }
    }
    else if (intent.state == NapkinState::FullBlank || intent.state == NapkinState::Full)
    {
        if (input.controls & controlBit(Controls::MouseButtonDown))
        {
            intent.state = NapkinState::Hidden;
            std::cout << "NapkinIntent: switched to Hidden (click inside full hitbox)\n";
        }
    }
}

void updateDragIntent(bagel::Entity e, UserInput input, AudioContext& audio)
{
    const WorldPos worldMouse =
        screenToWorldPoint(input.mousePos, RenderContext::getCameraPos());

    if (e.has<Falling>()) return;

    auto& intent = e.get<DragIntent>();

    if ((input.controls & controlBit(Controls::MouseButtonDown)) &&
        intent.intentType == DragIntentType::None &&
        isPointInsideTransform(worldMouse, e.get<Transform>()))
    {
        intent.intentType = DragIntentType::held;
        intent.mousePos   = input.mousePos;
        audio.play(sound::GRAB, sound::GRAB_VOLUME);
    }
    else if ((input.controls & controlBit(Controls::MouseButtonUp)) &&
             intent.intentType == DragIntentType::held)
    {
        intent.intentType = DragIntentType::released;
        intent.mousePos   = input.mousePos;
    }
    else if ((input.controls & controlBit(Controls::MouseMotion)) &&
             intent.intentType == DragIntentType::held)
    {
        // Keep the held entity tracking the cursor while dragging.
        intent.mousePos = input.mousePos;
    }
}


// Maps a liquid pipe to the key that pours it (LiquidIngredient order).
constexpr SDL_Scancode scancodeForIngredient(LiquidIngredient kind)
{
    switch (kind)
    {
    case LiquidIngredient::Coffee: return SDL_SCANCODE_1;
    case LiquidIngredient::Water:  return SDL_SCANCODE_2;
    case LiquidIngredient::Milk:   return SDL_SCANCODE_3;
    default:                 return SDL_SCANCODE_UNKNOWN;
    }
}

// Pours via the keyboard by driving the matching Machine button's `pressed`
// flag — the same persistent flag the mouse uses — so keyboard and mouse pour
// coexist and a held key survives across frames (SDL key events are edge-only).
// Locked until the KeyboardPour upgrade is owned.
void updateKeyboardPourIntent(bagel::Entity e, UserInput& input)
{
    if (UpgradeState::level(UpgradeId::KeyboardPour) < 1) return;

    Button& b = e.get<Button>();
    if (b.kind != ButtonKind::Machine) return;

    const SDL_Scancode myKey = scancodeForIngredient(b.liquid);
    for (SDL_Scancode sc : input.keyDowns)
        if (sc == myKey) b.pressed = true;
    for (SDL_Scancode sc : input.keyUps)
        if (sc == myKey) b.pressed = false;
}

}
void intentSystem(SDL_Renderer* renderer, bool& outExitCalled, AudioContext& audio)
{
    constexpr int RESERVED_POLL_KEY_EVENTS = 10;
    UserInput input{};
    input.keyUps.reserve(RESERVED_POLL_KEY_EVENTS);
    input.keyDowns.reserve(RESERVED_POLL_KEY_EVENTS);

    // gather events into out input object
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            outExitCalled = true;
            return;

        case SDL_EVENT_KEY_DOWN:
            input.controls |= controlBit(Controls::KeyDown);
            input.keyDowns.push_back(event.key.scancode);
            break;

        case SDL_EVENT_KEY_UP:
            input.controls |= controlBit(Controls::KeyUp);
            input.keyUps.push_back(event.key.scancode);
            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            input.controls |= controlBit(Controls::MouseButtonDown);
            input.mousePos = mouseWindowToRenderPoint(renderer,
                                                      event.button.x,
                                                      event.button.y);
            input.mouseButton = event.button.button;
            break;

        case SDL_EVENT_MOUSE_BUTTON_UP:
            input.controls |= controlBit(Controls::MouseButtonUp);
            input.mousePos = mouseWindowToRenderPoint(renderer,
                                                      event.button.x,
                                                      event.button.y);
            input.mouseButton = event.button.button;
            break;

        case SDL_EVENT_MOUSE_MOTION:
            input.controls |= controlBit(Controls::MouseMotion);
            input.mousePos = mouseWindowToRenderPoint(renderer,
                                                      event.motion.x,
                                                      event.motion.y);
            break;

        default:
            break;
        }
    }

    float wx, wy;
    SDL_GetMouseState(&wx, &wy);
    input.mousePos = mouseWindowToRenderPoint(renderer, wx, wy);

    // Single shared hit-test for every Button entity (Machine kind is the only
    // held one here; Spawn/Menu/Shop/Sound don't live in this scene's mask but
    // updateButtonsFromMouse handles them uniformly regardless).
    const WorldPos worldMouse =
        screenToWorldPoint(input.mousePos, RenderContext::getCameraPos());
    updateButtonsFromMouse(worldMouse,
                           input.controls & controlBit(Controls::MouseButtonDown),
                           input.controls & controlBit(Controls::MouseButtonUp),
                           audio);

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        static const bagel::Mask dragMask         = bagel::MaskBuilder().set<DragIntent>().set<Transform>().build();
        static const bagel::Mask buttonMask       = bagel::MaskBuilder().set<Button>().build();
        static const bagel::Mask napkinIntentMask = bagel::MaskBuilder().set<NapkinIntent>().build();

        if (e.test(dragMask))
            updateDragIntent(e, input, audio);

        if (e.test(buttonMask))
            updateKeyboardPourIntent(e, input);

        if (e.test(napkinIntentMask))
            updateNapkinIntent(e, input);
    }
}
} // namespace cafe
