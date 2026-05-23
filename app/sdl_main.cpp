#include <SDL3/SDL.h>
#include <iostream>

int main()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "Init error : " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }

    auto window = SDL_CreateWindow("MCE", 720, 405, SDL_WINDOW_OPENGL);

    if (!window)
    {
        std::cerr << "Window creation error : " << SDL_GetError() << std::endl;
        SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_SetWindowPosition(window,
                          SDL_WINDOWPOS_CENTERED,
                          SDL_WINDOWPOS_CENTERED);

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
