#include "IntentSystem.h"
#include "Components.h"
#include "RenderContext.h"
#include "Transform.h"
#include "UserInput.h"

#include <bagel.h>
#include <iostream>

namespace cafe
{
namespace
{
bool isPointInsideTransform(const WorldPos& p, const Transform& t)
{
    return p.x > t.x - t.w && p.x < t.x + t.w &&
           p.y > t.y - t.h && p.y < t.y + t.h;
}

void updateDragIntent(bagel::Entity e, UserInput input)
{
    const WorldPos worldMouse =
        screenToWorldPoint(input.mousePos, RenderContext::getCameraPos());

    auto& intent = e.get<DragIntent>();

    if ((input.controls & controlBit(Controls::MouseButtonDown)) &&
        intent.intentType == DragIntentType::None &&
        isPointInsideTransform(worldMouse, e.get<Transform>()))
    {
        intent.intentType = DragIntentType::held;
        intent.mousePos   = input.mousePos;
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


// Maps a liquid pipe to the key that pours it (Ingredient order).
constexpr SDL_Scancode scancodeForIngredient(Ingredient kind)
{
    switch (kind)
    {
    case Ingredient::Coffee: return SDL_SCANCODE_1;
    case Ingredient::Water:  return SDL_SCANCODE_2;
    case Ingredient::Milk:   return SDL_SCANCODE_3;
    default:                 return SDL_SCANCODE_UNKNOWN;
    }
}

// Sets the pour state of the pipe whose key matches sc (no-op for other keys).
void updatePipePourIntent(bagel::Entity e, UserInput& input)
{
    const SDL_Scancode myKey = scancodeForIngredient(e.get<LiquidSpawner>().kind);
    bool& spawnerActive = e.get<LiquidSpawner>().active;

    for (SDL_Scancode sc : input.keyDowns)
        if (sc == myKey) spawnerActive = true;
    for (SDL_Scancode sc : input.keyUps)
        if (sc == myKey) spawnerActive = false;
}

void updateMachineButtonIntent(bagel::Entity e, UserInput& input)
{
    auto& button = e.get<MachineButton>();
    const WorldPos worldMouse =
        screenToWorldPoint(input.mousePos, RenderContext::getCameraPos());

    if (input.controls & controlBit(Controls::MouseButtonDown) &&
        isPointInsideTransform(worldMouse, e.get<Transform>()))
    {
        button.pressed = true;
        std::cout << "started pressing" << std::endl;
    }
    else if (input.controls & controlBit(Controls::MouseButtonUp))
    {
        button.pressed = false;
        std::cout << "stopped  pressing" << std::endl;
    }
}

}
void intentSystem(SDL_Renderer* renderer, bool& outExitCalled)
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


    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {

        static const bagel::Mask dragMask       = bagel::MaskBuilder().set<DragIntent>().set<Transform>().build();
        static const bagel::Mask liqSpawnerMask = bagel::MaskBuilder().set<LiquidSpawner>().build();
        static const bagel::Mask machineButton  = bagel::MaskBuilder().set<MachineButton>().build();

        if (e.test(dragMask))
            updateDragIntent(e, input);

        if (e.test(liqSpawnerMask))
            updatePipePourIntent(e, input);

        if (e.test(machineButton))
            updateMachineButtonIntent(e, input);

        //test other intent masks, inputs, and transforms
    }
}
} // namespace cafe
