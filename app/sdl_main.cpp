#include "Components.h"
#include "GameConfig.h"
#include "RenderContext.h"
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

    // TODO: put it in a wrapper class with load fucntion
    SDL_Texture* bgTex = IMG_LoadTexture(renderer, "res/bg.png");
    assertFatal(bgTex != nullptr, SDL_GetError());
    float bgW{}, bgH{};
    SDL_GetTextureSize(bgTex, &bgW, &bgH);

    SDL_Texture* counterTex = IMG_LoadTexture(renderer, "res/counter.png");
    assertFatal(counterTex != nullptr, SDL_GetError());
    float counterW{}, counterH{};
    SDL_GetTextureSize(counterTex, &counterW, &counterH);

    Transform counterTransform{
        .x = 0.f,
        // line up the bottom of the counter with the bottom of the screen.
        .y = counterH / (2 * PTM) - LOGICAL_H / (2.f * PTM),
        .w = counterW / (2 * PTM),
        .h = counterH / (2 * PTM)};


    SDL_Texture* customerTex = IMG_LoadTexture(renderer, "res/def_customer.png");
    assertFatal(customerTex != nullptr, SDL_GetError());
    float customerW{}, customerH{};
    SDL_GetTextureSize(customerTex, &customerW, &customerH);
    Transform customerTransform{.x = 5.f,
                                .y = -0.5f,
                                .w = customerW / (2 * PTM),
                                .h = customerH / (2 * PTM)};

    SDL_FRect customerSrcRect{.x = 0, .y = 0, .w = customerW, .h = customerH};

    SDL_FRect counterSrcRect{.x = 0, .y = 0, .w = counterW, .h = counterH};
    SDL_FRect counterDstRect = transformToFrect(counterTransform, {0.f, 0.f});

    SDL_FRect bgSrcRect{.x = 0, .y = 0, .w = bgW, .h = bgH};
    SDL_FRect bgDstRect = {.x = 0, .y = 0, .w = LOGICAL_W, .h = LOGICAL_H};

    bool isRunning = true;

    while (isRunning)
    {
        // render here
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, bgTex, &bgSrcRect, &bgDstRect);
        customerTransform.x -= 0.01f;
        auto customerDstRect = transformToFrect(customerTransform, {0.f, 0.f});
        SDL_RenderTexture(renderer, customerTex, &customerSrcRect, &customerDstRect);
        SDL_RenderTexture(renderer, counterTex, &counterSrcRect, &counterDstRect);
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
    }

    SDL_Quit();

    return EXIT_SUCCESS;
}