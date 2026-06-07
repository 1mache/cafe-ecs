#include "CafeGame.h"
#include "Components.h"
#include "Systems.h"
#include "Utils.h"
#include "RenderContext.h"

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

    SDL_SetDefaultTextureScaleMode(renderer,SDL_SCALEMODE_NEAREST);
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    RenderContext::init(window, renderer);

    _window = window;
    _renderer = renderer;
}
} // namespace cafe