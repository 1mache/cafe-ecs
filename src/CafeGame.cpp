#include "CafeGame.h"

#include "CafeEnvironmentFactory.h"
#include "Components.h"
#include "Entities.h"
#include "GameConfig.h"
#include "PastryFactory.h"
#include "PhysicsContext.h"
#include "RenderContext.h"
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

    createCoffeeMachine(assets, {-4.f, 1.f}, {0.f, -0.5f});

    createCup(assets, {-4.f, -1.f}, CUP_CAPACITY);
    createPastry({4.f, -3.f},  assets);

    // Cleanup zone: off-screen sensor destroys spilled drops.
    createCleanupZone();


    // --- Customer ---
    const Order clientOrder{ .ratio = {3, 7, 0}, .hasDrink = true, .hasPastry = true };
    createClient(customerTex, customerW, customerH,
                                { 3.f, -1.f }, clientOrder, 60.f,
                                CUSTOMER_MOUTH_OFFSET);
}

void CafeGame::run()
{

    constexpr float PROPS_FRAME_W = 16.f, PROPS_FRAME_H = 16.f;

    // --- Client ---
    constexpr SDL_FPoint CUSTOMER_MOUTH_OFFSET_PX = {-10.f, -1.3f};
    Order sampleOrder{.ratio = {3, 7, 0}, .hasDrink = true, .hasPastry = true};
    auto  customerEnt = createClient(getAssetManager(), {5.f, -0.5f}, sampleOrder, 30.f,
                                     CUSTOMER_MOUTH_OFFSET_PX);

    // --- Speech bubble (child of client) ---
    constexpr float      BUBBLE_DISPLAY_W      = 24.f, BUBBLE_DISPLAY_H = 14.f;
    constexpr SDL_FPoint BUBBLE_TAIL_OFFSET_PX = {7.5f, -6.5f};

    SDL_FPoint mouth       = customerEnt.get<SpeechAnchor>().mouthOffset;
    SDL_FPoint bubbleOffPx = {mouth.x - BUBBLE_TAIL_OFFSET_PX.x,
                              mouth.y - BUBBLE_TAIL_OFFSET_PX.y + 1.f};
    auto       bubble      = createSpeechBubble(getAssetManager(), BUBBLE_DISPLAY_W, BUBBLE_DISPLAY_H,
                                                customerEnt, bubbleOffPx);
    bubble.get<Drawable>().renderLayer = LAYER_UI;

    // --- Order icons (children of the bubble) ---
    constexpr float ICON_SIZE = 8.f / 1.5f;
    constexpr float ICON_DX   = 3.f;
    constexpr float ICON_DY   = 2.f;
    if (sampleOrder.hasPastry)
    {
        SDL_FRect pastrySrc  = {0.f, 0.f, PROPS_FRAME_W, PROPS_FRAME_H};
        auto      pastryIcon = createOrderIcon(getAssetManager(), pastrySrc, ICON_SIZE, ICON_SIZE, bubble, {-ICON_DX, ICON_DY});
        pastryIcon.get<Drawable>().renderLayer = LAYER_UI;
    }
    if (sampleOrder.hasDrink)
    {
        SDL_FRect drinkSrc  = {0.f, 0.f, PROPS_FRAME_W, PROPS_FRAME_H}; // TODO: aim at drink frame in props.png
        auto      drinkIcon = createOrderIcon(getAssetManager(), drinkSrc, ICON_SIZE, ICON_SIZE, bubble, {ICON_DX, ICON_DY});
        drinkIcon.get<Drawable>().renderLayer = LAYER_UI;
    }

    // --- Coffee machine, cup, cleanup zone ---
    auto cupEnt = createCup(getAssetManager(), {-4.f, -1.f}, 50);
    (void)        createCleanupZone();

    bool   isRunning = true;
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
            case SDL_EVENT_KEY_DOWN:
                if (event.key.scancode == SDL_SCANCODE_SPACE
                    && !machineEnt.get<CoffeeSpawner>().active)
                {
                    machineEnt.get<CoffeeSpawner>().active = true;
                    std::cout << "[Pour] ON" << std::endl;
                }
                break;
            case SDL_EVENT_KEY_UP:
                if (event.key.scancode == SDL_SCANCODE_SPACE
                    && machineEnt.get<CoffeeSpawner>().active)
                {
                    machineEnt.get<CoffeeSpawner>().active = false;
                    std::cout << "[Pour] OFF" << std::endl;
                }
                break;
            }
        }

        behaviorSystem(dt);
        coffeeSpawnerSystem(dt, getAssetManager());
        PhysicsContext::step(dt);
        sensorEventSystem();
        syncTransformFromBody();
        hierarchySystem();
        orderSystem();
        cleanupSystem();
        dumpDebugStatsEvery(dt);

        SDL_RenderClear(_renderer);
        drawSystem(_renderer);
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
