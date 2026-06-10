#include "CafeGame.h"

#include "Components.h"
#include "Entities.h"
#include "GameConfig.h"
#include "PhysicsContext.h"
#include "RenderContext.h"
#include "SpriteDims.h"
#include "Systems.h"
#include "Utils.h"
#include <SDL3/SDL.h>
#include <iostream>

namespace cafe
{

void CafeGame::init()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "Init error : " << SDL_GetError() << std::endl;
        fatalError(SDL_GetError());
    }

    SDL_Window*   window{};
    SDL_Renderer* renderer{};
    SDL_CreateWindowAndRenderer(WINDOW_TITLE,
                                static_cast<int>(START_WIN_W),
                                static_cast<int>(START_WIN_H),
                                SDL_WINDOW_OPENGL,
                                &window,
                                &renderer);

    if (!window)
    {
        std::cerr << "Window creation error : " << SDL_GetError() << std::endl;
        SDL_Quit();
        fatalError(SDL_GetError());
    }
    if (!renderer)
    {
        std::cerr << "Renderer creation error : " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        fatalError(SDL_GetError());
    }

    SDL_SetDefaultTextureScaleMode(renderer, SDL_SCALEMODE_NEAREST);
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    RenderContext::init(window, renderer);

    _window   = window;
    _renderer = renderer;

    _currentScene.init(_renderer);
    PhysicsContext::init();

    auto& assets = getAssetManager();
    createBg(_currentScene.getBgTexture());
    createBartop(assets);

    _machineEnt = createCoffeeMachine(assets, {-6.f, -1.f}, {0.f, -0.5f});

    createCup(assets, {-4.f, -1.f}, CUP_CAPACITY);
    createPastry({4.f, -3.f},  assets);

    // Cleanup zone: off-screen sensor destroys spilled drops.
    createCleanupZone();


    // --- Customer ---
    Order customerOrder{ .ratio = {3, 7, 0}, .hasDrink = true, .hasPastry = true };
    auto customerEnt = createCustomer(assets, { 5.f, -1.f }, customerOrder, 60.f);

    // --- Speech bubble + order icons (children of customer) ---
    auto bubbleEnt = createSpeechBubble(assets, customerEnt, {-PERSON_DIMS.x, PERSON_DIMS.y * 0.25f});
    // --- Order icons (children of the bubble) ---
    constexpr float ICON_SIZE = 8.f / 1.5f; // TODO:move somewhere else
    constexpr float ICON_DX   = 3.f;
    constexpr float ICON_DY   = 2.f;
    if (customerOrder.hasDrink)
    {
        // Drink icon (right) — front frame of big_cup (16x16).
        createOrderIcon(assets,
                        2,
                        ICON_SIZE,
                        ICON_SIZE,
                        bubbleEnt,
                        {ICON_DX, ICON_DY});
    }
    if (customerOrder.hasPastry)
    {
        // Pastry icon (left) — frame 0 of props strip.
        createOrderIcon(assets,
                        0,
                        ICON_SIZE,
                        ICON_SIZE,
                        bubbleEnt,
                        {-ICON_DX, ICON_DY});
    }
}

void CafeGame::run()
{

    std::cout << "[Demo] Hold SPACE to pour coffee. Left-drag to move the cup or pastry.\n"
              << "[Demo] Serve a FULL cup + pastry to the customer in 60 s.\n";

    bool   isRunning = true;
    bool   isDragging = false;
    Uint64 lastTicks = SDL_GetTicks();

    while (isRunning)
    {
        const auto frameStart = SDL_GetTicks();
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

                // TODO:move into input system
                case SDL_EVENT_KEY_DOWN:
                    if (event.key.scancode == SDL_SCANCODE_SPACE
                        && !_machineEnt.get<CoffeeSpawner>().active)
                    {
                        _machineEnt.get<CoffeeSpawner>().active = true;
                        std::cout << "[Pour] ON\n";
                    }
                    break;

                case SDL_EVENT_KEY_UP:
                    if (event.key.scancode == SDL_SCANCODE_SPACE
                        && _machineEnt.get<CoffeeSpawner>().active)
                    {
                        _machineEnt.get<CoffeeSpawner>().active = false;
                        std::cout << "[Pour] OFF\n";
                    }
                    break;

                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    if (event.button.button == SDL_BUTTON_LEFT)
                    {
                        const SDL_FPoint pos =
                            mouseWindowToRenderPoint(_renderer,
                                                     event.button.x,
                                                     event.button.y);
                        dropSpacePickupSystem(pos);
                        dragStartSystem(pos);
                        enableSensorEventsOnHeldEntities();
                        isDragging = true;
                    }
                    break;

                case SDL_EVENT_MOUSE_MOTION:
                    if (isDragging)
                    {
                        const SDL_FPoint pos =
                            mouseWindowToRenderPoint(_renderer,
                                                     event.motion.x,
                                                     event.motion.y);
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

            coffeeSpawnerSystem(dt, getAssetManager());    // spawn drops while pouring
            PhysicsContext::step(dt);
            sensorEventSystem();        // count drops into cup; cleanup spilled
            dropSpaceDetectionSystem(); // update Held.dropSpaceEntity
            syncTransformFromBody();    // physics position -> Transform

            behaviorSystem(dt);         // tick patience; adds Leaving on timeout (fail)
            orderSystem();              // full cup + pastry -> rating=1 + Leaving (success)
            reportLeavingCustomers();   // log SUCCESSFUL / FAILED
            hierarchySystem();          // children follow parents; orphan children of Leaving
            cleanupSystem();            // destroy all Leaving entities

            SDL_RenderClear(_renderer);
            drawSystem(_renderer);       // sorted by renderLayer ascending
            SDL_RenderPresent(_renderer);

        constexpr Uint64 frameDeltaT = static_cast<Uint64>(FRAME_DELTA_MS);
        auto             frameEnd    = SDL_GetTicks();
        if (frameEnd - frameStart < frameDeltaT)
            SDL_Delay(static_cast<Uint32>(frameDeltaT - (frameEnd - frameStart)));
    }
}

void CafeGame::destroy() const
{
    PhysicsContext::shutdown();
    SDL_DestroyRenderer(_renderer);
    SDL_DestroyWindow(_window);
    SDL_Quit();
}

} // namespace cafe
