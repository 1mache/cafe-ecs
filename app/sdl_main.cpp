#include "GameConfig.h"
#include "RenderContext.h"
#include <SDL3/SDL.h>
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

    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    RenderContext::init(window, renderer);

    bool isRunning = true;

    while (isRunning)
    {
        // render here

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