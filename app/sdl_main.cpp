#include "Components.h"
#include "Entities.h"
#include "Systems.h"
#include "GameConfig.h"
#include "RenderContext.h"
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

    // TODO: put it in a wrapper class with load fucntion
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

    auto bgEnt = bagel::Entity::create();
    bgEnt.addAll(
        Drawable{bgTex, {0.f, 0.f, bgW, bgH}},
        Transform{.x = 0.f, .y = 0.f, .w = LOGICAL_W / (2.f * PTM), .h = LOGICAL_H / (2.f * PTM)});

    Order sampleOrder{.ratio = {3, 7, 0}, .hasDrink = true, .hasPastry = true};
    auto  customerEnt = createClient(customerTex, customerW, customerH,
                                     {5.f, -0.5f}, sampleOrder, 30.f);

    auto counterEnt = bagel::Entity::create();
    counterEnt.addAll(
        Drawable{counterTex, {0.f, 0.f, counterW, counterH}},
        Transform{.x  = 0.f,
                  .y  = counterH / (2 * PTM) - LOGICAL_H / (2.f * PTM),
                  .w  = counterW / (2 * PTM),
                  .h  = counterH / (2 * PTM)});

    bool isRunning = true;

    while (isRunning)
    {
        auto frameStart = SDL_GetTicks();

        // render here
        SDL_RenderClear(renderer);
        customerEnt.get<Transform>().x -= 0.01f;
        constexpr float dt = FRAME_DELTA_MS / 1000.f;
        behaviorSystem(dt);
        orderSystem();
        drawSystem();
        SDL_RenderPresent(renderer);
        // input
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

        frameStart += frameDeltaT;
    }

    SDL_Quit();

    return EXIT_SUCCESS;
}