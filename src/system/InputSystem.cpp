#include "InputSystem.h"
#include "SdlEvents.h"
#include <bagel.h>

namespace cafe
{
namespace
{
constexpr uint32_t controlBit(Controls control)
{
    return 1u << static_cast<int>(control);
}
} // namespace

void eventGatheringSystem(SDL_Renderer* renderer)
{
    SdlEvents frameEvents{};

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            frameEvents.controls |= controlBit(Controls::Quit);
            break;

        case SDL_EVENT_KEY_DOWN:
            frameEvents.controls |= controlBit(Controls::KeyDown);
            frameEvents.keyScancode = event.key.scancode;
            break;

        case SDL_EVENT_KEY_UP:
            frameEvents.controls |= controlBit(Controls::KeyUp);
            frameEvents.keyScancode = event.key.scancode;
            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            frameEvents.controls |= controlBit(Controls::MouseButtonDown);
            frameEvents.mousePos = mouseWindowToRenderPoint(renderer,
                                                            event.button.x,
                                                            event.button.y);
            frameEvents.mouseButton = event.button.button;
            break;

        case SDL_EVENT_MOUSE_BUTTON_UP:
            frameEvents.controls |= controlBit(Controls::MouseButtonUp);
            frameEvents.mousePos = mouseWindowToRenderPoint(renderer,
                                                            event.button.x,
                                                            event.button.y);
            frameEvents.mouseButton = event.button.button;
            break;

        case SDL_EVENT_MOUSE_MOTION:
            frameEvents.controls |= controlBit(Controls::MouseMotion);
            frameEvents.mousePos = mouseWindowToRenderPoint(renderer,
                                                            event.motion.x,
                                                            event.motion.y);
            break;
        }
    }

    static const bagel::Mask mask = bagel::MaskBuilder().set<SdlEvents>().build();
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask)) continue;
        e.get<SdlEvents>() = frameEvents;
    }
}

} // namespace cafe
