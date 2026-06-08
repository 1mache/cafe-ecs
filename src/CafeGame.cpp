#include "CafeGame.h"
#include "Components.h"
#include "Entities.h"
#include "GameConfig.h"
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
}

void CafeGame::run()
{
    auto& bgTex     = _currentScene.getBgTexture();
    auto  bgSrcRect = bgTex.getFullSrcRect();

    auto& bartopTex     = getAssetManager().getTexture("counter.png");
    auto  bartopSrcRect = bartopTex.getFullSrcRect();

    auto& customerTex = getAssetManager().getTexture("def_customer.png");
    float customerW = customerTex.getSize().x, customerH = customerTex.getSize().y;

    auto&           cupTex      = getAssetManager().getTexture("big_cup.png");
    constexpr float CUP_FRAME_W = 24.f, CUP_FRAME_H = 24.f;

    auto&           propsTex      = getAssetManager().getTexture("props.png");
    constexpr float PROPS_FRAME_W = 16.f, PROPS_FRAME_H = 16.f;


    auto& bubbleTex = getAssetManager().getTexture("bubble.png");
    float bubbleW = bubbleTex.getSize().x, bubbleH = bubbleTex.getSize().y;

    // --- Background ---
    auto bgEnt = bagel::Entity::create();
    bgEnt.addAll(Drawable{bgTex.get(), bgSrcRect},
                 Transform{.x = 0.f,
                           .y = 0.f,
                           .w = LOGICAL_W / (2.f * PTM),
                           .h = LOGICAL_H / (2.f * PTM)});

    // --- Counter ---
    auto  bartopEnt        = bagel::Entity::create();
    float bartopHalfHeight = screenToWorldSize(bartopSrcRect.h / 2.f);
    bartopEnt.addAll(
        Drawable{bartopTex.get(), bartopSrcRect},
        Transform{.x = 0.f,
                  .y = -(screenToWorldSize(LOGICAL_H / 2) - bartopHalfHeight),
                  .w = screenToWorldSize(bartopSrcRect.w / 2.f),
                  .h = bartopHalfHeight});

    // --- Client ---
    // Mouth offset from sprite center, in logical pixels (Y-up).
    constexpr SDL_FPoint CUSTOMER_MOUTH_OFFSET_PX = {-4.f, -1.3f};
    Order sampleOrder{.ratio = {3, 7, 0}, .hasDrink = true, .hasPastry = true};
    auto  customerEnt = createClient(customerTex.get(),
                                    customerW,
                                    customerH,
                                     {5.f, -0.5f},
                                    sampleOrder,
                                    30.f,
                                    CUSTOMER_MOUTH_OFFSET_PX);

    // --- Speech bubble (child of client) ---
    // Tail tip is at the bottom-right of the bubble sprite (measured from raw 48x24 pixels).
    // BUBBLE_TAIL_OFFSET: offset from bubble center to its tail tip, in logical px (Y-up).
    constexpr float      BUBBLE_DISPLAY_W = 24.f, BUBBLE_DISPLAY_H = 14.f;
    constexpr SDL_FPoint BUBBLE_TAIL_OFFSET_PX = {7.5f, -6.5f};

    // Shift bubble so its tail tip lands on the client's mouth. 1px gap keeps tail visible.
    SDL_FRect  bubbleSrc   = {0.f, 0.f, bubbleW, bubbleH};
    SDL_FPoint mouth       = customerEnt.get<SpeechAnchor>().mouthOffset;
    SDL_FPoint bubbleOffPx = {mouth.x - BUBBLE_TAIL_OFFSET_PX.x,
                              mouth.y - BUBBLE_TAIL_OFFSET_PX.y + 1.f};
    auto       bubble      = createSpeechBubble(bubbleTex.get(),
                                     bubbleSrc,
                                     BUBBLE_DISPLAY_W,
                                     BUBBLE_DISPLAY_H,
                                     customerEnt,
                                     bubbleOffPx);

    // --- Order icons (children of the bubble) ---
    // props.png is a 3-frame strip: [cinnamon roll | croissant | cup], each propsW/3 wide.
    constexpr float ICON_SIZE = 8.f / 1.5f;
    constexpr float ICON_DX   = 3.f;
    constexpr float ICON_DY   = 2.f;
    if (sampleOrder.hasPastry)
    {
        SDL_FRect pastrySrc = {0.f, 0.f, PROPS_FRAME_W, PROPS_FRAME_H};
        createOrderIcon(propsTex.get(),
                        pastrySrc,
                        ICON_SIZE,
                        ICON_SIZE,
                        bubble,
                        {-ICON_DX, ICON_DY});
    }
    if (sampleOrder.hasDrink)
    {
        SDL_FRect drinkSrc = {2.f * PROPS_FRAME_W,
                              0.f,
                              PROPS_FRAME_W,
                              PROPS_FRAME_H};
        createOrderIcon(propsTex.get(),
                        drinkSrc,
                        ICON_SIZE,
                        ICON_SIZE,
                        bubble,
                        {ICON_DX, ICON_DY});
    }

    bool isRunning = true;

    while (isRunning)
    {
        auto frameStart = SDL_GetTicks();

        SDL_RenderClear(_renderer);

        customerEnt.get<Transform>().x -= 0.01f;

        constexpr float dt = FRAME_DELTA_MS / 1000.f;
        behaviorSystem(dt);
        hierarchySystem();
        orderSystem();
        cleanupSystem();
        drawSystem();

        SDL_RenderPresent(_renderer);

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
                isRunning = false;
                break;
            }
        }

        constexpr auto frameDeltaT = static_cast<Uint32>(FRAME_DELTA_MS);
        auto           frameEnd    = SDL_GetTicks();
        if (frameEnd - frameStart < frameDeltaT)
            SDL_Delay(frameDeltaT - static_cast<Uint32>((frameEnd - frameStart)));
    }

    SDL_Quit();
}

} // namespace cafe
