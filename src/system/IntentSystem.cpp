#include "IntentSystem.h"
#include "Components.h"
#include "RenderContext.h"
#include "Transform.h"
#include <bagel.h>

namespace cafe
{
namespace
{
constexpr uint32_t controlBit(Controls control)
{
    return 1u << static_cast<int>(control);
}

bool pointInTransform(const WorldPos& p, const Transform& t)
{
    return p.x > t.x - t.w && p.x < t.x + t.w &&
           p.y > t.y - t.h && p.y < t.y + t.h;
}

// Maps a liquid pipe to the key that pours it (Ingredient order).
constexpr SDL_Scancode scancodeForIngredient(Ingredient kind)
{
    switch (kind)
    {
    case Ingredient::Coffee: return SDL_SCANCODE_1;
    case Ingredient::Milk:   return SDL_SCANCODE_2;
    case Ingredient::Water:  return SDL_SCANCODE_3;
    default:                 return SDL_SCANCODE_UNKNOWN;
    }
}

// Sets the pour state of the pipe whose key matches sc (no-op for other keys).
void setPipePour(SDL_Scancode sc, bool active)
{
    static const bagel::Mask mask = bagel::MaskBuilder().set<LiquidSpawner>().build();
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (e.test(mask) && scancodeForIngredient(e.get<LiquidSpawner>().kind) == sc)
            e.get<LiquidSpawner>().active = active;
    }
}

void publishSdlEvents(const SdlEvents& input)
{
    static const bagel::Mask mask = bagel::MaskBuilder().set<SdlEvents>().build();
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (e.test(mask))
            e.get<SdlEvents>() = input;
    }
}
} // namespace

void intentSystem(SDL_Renderer* renderer)
{
    // 1. Poll SDL events into a per-frame mask (one bit per Controls value).
    //    Payload of the last event of each kind is kept for future intents.
    SdlEvents input{};

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            input.controls |= controlBit(Controls::Quit);
            break;

        case SDL_EVENT_KEY_DOWN:
            input.controls |= controlBit(Controls::KeyDown);
            input.keyScancode = event.key.scancode;
            setPipePour(event.key.scancode, true);  // hold 1/2/3 to pour
            break;

        case SDL_EVENT_KEY_UP:
            input.controls |= controlBit(Controls::KeyUp);
            input.keyScancode = event.key.scancode;
            setPipePour(event.key.scancode, false);
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
        }
    }

    // 2. Expose the per-frame input to other systems via the SdlEvents
    //    input-state entity (Quit, key, and mouse payloads).
    publishSdlEvents(input);

    // 3. Translate the per-frame input into per-entity DragIntent transitions.
    const WorldPos worldMouse =
        screenToWorldPoint(input.mousePos, RenderContext::getCameraPos());

    static const bagel::Mask dragMask = bagel::MaskBuilder().set<DragIntent>().set<Transform>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (e.test(dragMask)){
            // Items mid-drop (Falling) or already handed over (DeliveredTo) can't be grabbed.
            if (e.has<Falling>() || e.has<DeliveredTo>()) continue;

            auto& intent = e.get<DragIntent>();

            if ((input.controls & controlBit(Controls::MouseButtonDown))
                && intent.intentType == DragIntentType::None
                && pointInTransform(worldMouse, e.get<Transform>())){
                intent.intentType = DragIntentType::held;
                intent.mousePos   = input.mousePos;
            }
            else if ((input.controls & controlBit(Controls::MouseButtonUp))
                     && intent.intentType == DragIntentType::held){
                intent.intentType = DragIntentType::released;
                intent.mousePos   = input.mousePos;
            }
            else if ((input.controls & controlBit(Controls::MouseMotion))
                     && intent.intentType == DragIntentType::held){
                // Keep the held entity tracking the cursor while dragging.
                intent.mousePos = input.mousePos;
            }
        }
        
        //test other intent masks, inputs, and transforms
    }
}
} // namespace cafe
