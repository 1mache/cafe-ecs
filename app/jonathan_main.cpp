#include "Components.h"
#include "Entities.h"
#include "GameConfig.h"
#include "RenderContext.h"
#include "Systems.h"
#include "TransformOperations.h"
#include "Utils.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

int main()
{
    using namespace cafe;

    // example of using nlohmann::json to read a json file
    {
        std::ifstream  infoFile("res/info.json");
        nlohmann::json info;
        infoFile >> info;
        std::cout << "message: " << info["message"].get<std::string>() << "\n"
                  << "location: " << info["location"].get<std::string>()
                  << std::endl;
    }

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "Init error : " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }

    SDL_Window*   window{};
    SDL_Renderer* renderer{};
    SDL_CreateWindowAndRenderer("Cafe",
                                static_cast<int>(START_WIN_W),
                                static_cast<int>(START_WIN_H),
                                SDL_WINDOW_OPENGL,
                                &window,
                                &renderer);

    if (!window)
    {
        std::cerr << "Window creation error : " << SDL_GetError() << std::endl;
        SDL_Quit();
        return EXIT_FAILURE;
    }
    if (!renderer)
    {
        std::cerr << "Renderer creation error : " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    // good for pixel art scaling
    SDL_SetDefaultTextureScaleMode(renderer, SDL_SCALEMODE_NEAREST);

    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    RenderContext::init(window, renderer);

    // TODO: put it in a wrapper class with load function
    SDL_Texture* bgTex = IMG_LoadTexture(renderer, "res/bg.png");
    assertFatal(bgTex != nullptr, SDL_GetError());
    float bgW{}, bgH{};
    SDL_GetTextureSize(bgTex, &bgW, &bgH);

    SDL_Texture* counterTex = IMG_LoadTexture(renderer, "res/counter.png");
    assertFatal(counterTex != nullptr, SDL_GetError());
    float counterW{}, counterH{};
    SDL_GetTextureSize(counterTex, &counterW, &counterH);

    SDL_Texture* customerTex = IMG_LoadTexture(renderer, "res/def_customer.png");
    assertFatal(customerTex != nullptr, SDL_GetError());
    float customerW{}, customerH{};
    SDL_GetTextureSize(customerTex, &customerW, &customerH);

    // Beverage icon: front frame of the layered cup sprite (24x24 per big_cup.json)
    SDL_Texture* cupTex = IMG_LoadTexture(renderer, "res/big_cup.png");
    assertFatal(cupTex != nullptr, SDL_GetError());
    constexpr float CUP_FRAME_W = 24.f, CUP_FRAME_H = 24.f;

    // Pastry icon: 3-frame strip [cinnamon roll | croissant | cup]; frame 0 used as pastry icon
    SDL_Texture* propsTex = IMG_LoadTexture(renderer, "res/props.png");
    assertFatal(propsTex != nullptr, SDL_GetError());
    float propsW{}, propsH{};
    SDL_GetTextureSize(propsTex, &propsW, &propsH);

    // Speech bubble
    SDL_Texture* bubbleTex = IMG_LoadTexture(renderer, "res/bubble.png");
    assertFatal(bubbleTex != nullptr, SDL_GetError());
    float bubbleW{}, bubbleH{};
    SDL_GetTextureSize(bubbleTex, &bubbleW, &bubbleH);

    // --- Background ---
    auto bgEnt = bagel::Entity::create();
    bgEnt.addAll(
        Drawable{bgTex, {0.f, 0.f, bgW, bgH}},
        Transform{.x = 0.f, .y = 0.f, .w = LOGICAL_W / (2.f * PTM), .h = LOGICAL_H / (2.f * PTM)});

    // --- Counter ---
    auto counterEnt = bagel::Entity::create();
    counterEnt.addAll(
        Drawable{counterTex, {0.f, 0.f, counterW, counterH}},
        Transform{.x  = 0.f,
                  .y  = counterH / (2 * PTM) - LOGICAL_H / (2.f * PTM),
                  .w  = counterW / (2 * PTM),
                  .h  = counterH / (2 * PTM)});

    // --- Client ---
    // Mouth offset from sprite center, in logical pixels (Y-up).
    constexpr SDL_FPoint CUSTOMER_MOUTH_OFFSET_PX = {-4.f, -1.3f};
    Order sampleOrder{.ratio = {3, 7, 0}, .hasDrink = true, .hasPastry = true};
    auto  customerEnt = createClient(customerTex, customerW, customerH,
                                     {5.f, -0.5f}, sampleOrder, 30.f,
                                     CUSTOMER_MOUTH_OFFSET_PX);

    // --- Speech bubble (child of client) ---
    // Tail tip is at the bottom-right of the bubble sprite (measured from raw 48x24 pixels).
    // BUBBLE_TAIL_OFFSET: offset from bubble center to its tail tip, in logical px (Y-up).
    constexpr float BUBBLE_DISPLAY_W = 24.f, BUBBLE_DISPLAY_H = 14.f; // logical px
    constexpr SDL_FPoint BUBBLE_TAIL_OFFSET_PX = {7.5f, -6.5f};

    // Shift bubble so its tail tip lands on the client's mouth. 1px gap keeps tail visible.
    SDL_FRect  bubbleSrc   = {0.f, 0.f, bubbleW, bubbleH};
    SDL_FPoint mouth       = customerEnt.get<SpeechAnchor>().mouthOffset;
    SDL_FPoint bubbleOffPx = {mouth.x - BUBBLE_TAIL_OFFSET_PX.x,
                              mouth.y - BUBBLE_TAIL_OFFSET_PX.y + 1.f};
    auto bubble = createSpeechBubble(bubbleTex, bubbleSrc,
                                     BUBBLE_DISPLAY_W, BUBBLE_DISPLAY_H,
                                     customerEnt, bubbleOffPx);

    // --- Order icons (children of the bubble) ---
    // props.png is a 3-frame strip: [cinnamon roll | croissant | cup], each propsW/3 wide.
    constexpr float ICON_SIZE = 8.f / 1.5f; // logical px, square
    constexpr float ICON_DX   = 3.f;        // horizontal spread from bubble center
    constexpr float ICON_DY   = 2.f;        // nudge up inside the bubble
    if (sampleOrder.hasPastry)
    {
        SDL_FRect pastrySrc = {0.f, 0.f, propsW / 3.f, propsH}; // frame 0 = cinnamon roll
        createOrderIcon(propsTex, pastrySrc, ICON_SIZE, ICON_SIZE, bubble, {-ICON_DX, ICON_DY});
    }
    if (sampleOrder.hasDrink)
    {
        SDL_FRect drinkSrc = {0.f, 0.f, CUP_FRAME_W, CUP_FRAME_H}; // big_cup front frame
        createOrderIcon(cupTex, drinkSrc, ICON_SIZE, ICON_SIZE, bubble, {ICON_DX, ICON_DY});
    }

    bool isRunning = true;

    while (isRunning)
    {
        auto frameStart = SDL_GetTicks();

        SDL_RenderClear(renderer);

        customerEnt.get<Transform>().x -= 0.01f;

        constexpr float dt = FRAME_DELTA_MS / 1000.f;
        behaviorSystem(dt);     // tick patience; marks expired clients Leaving
        hierarchySystem();      // move children to follow parents; drop children of Leaving parents
        orderSystem();
        cleanupSystem();        // destroy Leaving entities
        drawSystem();

        SDL_RenderPresent(renderer);

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
        if (frameEnd - frameStart < static_cast<Uint32>(frameDeltaT))
            SDL_Delay(static_cast<Uint32>(frameDeltaT - (frameEnd - frameStart)));
    }

    SDL_Quit();

    return EXIT_SUCCESS;
}
