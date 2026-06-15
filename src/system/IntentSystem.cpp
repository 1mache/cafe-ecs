#include "IntentSystem.h"
#include "Components.h"
#include "RenderContext.h"
#include "Transform.h"
#include "UserInput.h"

#include <bagel.h>

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

    // oif the scancode
    if (scancodeForIngredient(e.get<LiquidSpawner>().kind) == input.keyScancode)
    {
        bool& spawnerActive = e.get<LiquidSpawner>().active;
        if (input.controls & controlBit(Controls::MouseButtonDown))
            spawnerActive = true;
        if (input.controls & controlBit(Controls::MouseButtonUp))
            spawnerActive = false;
    }
}
}
void intentSystem(SDL_Renderer* renderer, bool& outExitCalled)
{
    UserInput input{};

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
            input.keyScancode = event.key.scancode;
            break;

        case SDL_EVENT_KEY_UP:
            input.controls |= controlBit(Controls::KeyUp);
            input.keyScancode = event.key.scancode;
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

        if (e.test(dragMask))
            updateDragIntent(e, input);

        if (e.test(liqSpawnerMask))
            updatePipePourIntent(e, input);

        //test other intent masks, inputs, and transforms
    }
}
} // namespace cafe
