#include "AssetManager.h"
#include "Assets.h"
#include "Components.h"
#include "Draggable.h"
#include "Entities.h"
#include "GameConfig.h"
#include "PhysicsContext.h"
#include "PhysicsFilters.h"
#include "RenderContext.h"
#include "Systems.h"
#include "Utils.h"
#include <SDL3/SDL.h>
#include <box2d/box2d.h>
#include <iostream>

namespace
{
// Speech-bubble geometry (measured from bubble.png, same as jonathan_main).
constexpr float      BUBBLE_DISPLAY_W      = 24.f;          // logical px
constexpr float      BUBBLE_DISPLAY_H      = 14.f;
constexpr SDL_FPoint BUBBLE_TAIL_OFFSET_PX = { 7.5f, -6.5f };
constexpr SDL_FPoint CUSTOMER_MOUTH_OFFSET = { -4.f, -1.3f };

// Order-icon layout inside the bubble.
constexpr float ICON_SIZE = 8.f / 1.5f; // ~5.33 logical px, square
constexpr float ICON_DX   = 3.f;
constexpr float ICON_DY   = 2.f;

// How many drops fill the cup.
constexpr int kCupCapacity = 50;

// ---- helper functions -------------------------------------------------------

SDL_FPoint mouseWindowToRenderPoint(float windowX, float windowY)
{
    SDL_FPoint p{ windowX, windowY };
    SDL_RenderCoordinatesFromWindow(cafe::RenderContext::getRenderer(),
                                    windowX, windowY, &p.x, &p.y);
    return p;
}

} // anonymous namespace

int main()
{
    {
        // --- Speech bubble + order icons (children of customer) ---
        {
            const SDL_FPoint mouth = customerEnt.get<SpeechAnchor>().mouthOffset;
            const SDL_FPoint bubbleOff = { mouth.x - BUBBLE_TAIL_OFFSET_PX.x,
                                           mouth.y - BUBBLE_TAIL_OFFSET_PX.y + 1.f };
            const SDL_FRect  bubbleSrc = { 0.f, 0.f, bubbleW, bubbleH };
            auto bubble = createSpeechBubble(bubbleTex, bubbleSrc,
                                             BUBBLE_DISPLAY_W, BUBBLE_DISPLAY_H,
                                             customerEnt, bubbleOff);
            bubble.get<Drawable>().renderLayer = kLayerBubble;

            // Pastry icon (left) — frame 0 of props strip.
            const SDL_FRect pastrySrc  = { 0.f, 0.f, propsW / 3.f, propsH };
            auto pastryIcon = createOrderIcon(propsTex, pastrySrc,
                                              ICON_SIZE, ICON_SIZE,
                                              bubble, { -ICON_DX, ICON_DY });
            pastryIcon.get<Drawable>().renderLayer = kLayerIcon;

            // Drink icon (right) — front frame of big_cup (24x24).
            const SDL_FRect drinkSrc = { 0.f, 0.f, 24.f, 24.f };
            auto drinkIcon = createOrderIcon(bigCupTex, drinkSrc,
                                             ICON_SIZE, ICON_SIZE,
                                             bubble, { ICON_DX, ICON_DY });
            drinkIcon.get<Drawable>().renderLayer = kLayerIcon;
        }

        std::cout << "[Demo] Hold SPACE to pour coffee. Left-drag to move the cup or pastry.\n"
                  << "[Demo] Serve a FULL cup + pastry to the customer in 60 s.\n";

        bool   isRunning  = true;
        bool   isDragging = false;
        Uint64 lastTicks  = SDL_GetTicks();

        while (isRunning)
        {
            const Uint64 frameStart = SDL_GetTicks();
            float dt = static_cast<float>(frameStart - lastTicks) * 0.001f;
            if (dt > 0.05f) dt = 0.05f;
            lastTicks = frameStart;

            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                switch (event.type)
                {
                case SDL_EVENT_QUIT:
                    isRunning = false;
                    break;

                case SDL_EVENT_KEY_DOWN:
                    if (event.key.scancode == SDL_SCANCODE_SPACE
                        && !machineEnt.get<CoffeeSpawner>().active)
                    {
                        machineEnt.get<CoffeeSpawner>().active = true;
                        std::cout << "[Pour] ON\n";
                    }
                    break;

                case SDL_EVENT_KEY_UP:
                    if (event.key.scancode == SDL_SCANCODE_SPACE
                        && machineEnt.get<CoffeeSpawner>().active)
                    {
                        machineEnt.get<CoffeeSpawner>().active = false;
                        std::cout << "[Pour] OFF\n";
                    }
                    break;

                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    if (event.button.button == SDL_BUTTON_LEFT)
                    {
                        const SDL_FPoint pos = mouseWindowToRenderPoint(
                            static_cast<float>(event.button.x),
                            static_cast<float>(event.button.y));
                        dropSpacePickupSystem(pos);
                        dragStartSystem(pos);
                        cafe::enableSensorEventsOnHeldEntities();
                        isDragging = true;
                    }
                    break;

                case SDL_EVENT_MOUSE_MOTION:
                    if (isDragging)
                    {
                        const SDL_FPoint pos = mouseWindowToRenderPoint(
                            static_cast<float>(event.motion.x),
                            static_cast<float>(event.motion.y));
                        midDragSystem(pos);
                    }
                    break;

                case SDL_EVENT_MOUSE_BUTTON_UP:
                    if (event.button.button == SDL_BUTTON_LEFT && isDragging)
                    {
                        // deliverySystem reads Held.dropSpaceEntity before
                        // dragReleaseSystem removes the Held component.
                        deliverySystem();
                        dragReleaseSystem();
                        isDragging = false;
                    }
                    break;
                }
            }

            coffeeSpawnerSystem(dt, Assets::particle());    // spawn drops while pouring
            PhysicsContext::step(dt);
            sensorEventSystem();        // count drops into cup; cleanup spilled
            dropSpaceDetectionSystem(); // update Held.dropSpaceEntity
            syncTransformFromBody();    // physics position -> Transform

            behaviorSystem(dt);         // tick patience; adds Leaving on timeout (fail)
            orderSystem();              // full cup + pastry -> rating=1 + Leaving (success)
            reportLeavingClients();     // log SUCCESSFUL / FAILED
            hierarchySystem();          // children follow parents; orphan children of Leaving
            cleanupSystem();            // destroy all Leaving entities

            SDL_RenderClear(renderer);
            drawSystem(renderer);       // sorted by renderLayer ascending
            SDL_RenderPresent(renderer);

            constexpr Uint64 frameDeltaT = static_cast<Uint64>(FRAME_DELTA_MS);
            const    Uint64  frameEnd    = SDL_GetTicks();
            if (frameEnd - frameStart < frameDeltaT)
                SDL_Delay(static_cast<Uint32>(frameDeltaT - (frameEnd - frameStart)));
        }

    } // AssetManager freed here — before renderer is destroyed.

    PhysicsContext::shutdown();
    Assets::shutdown();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}