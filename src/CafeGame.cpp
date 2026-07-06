#include "CafeGame.h"

#include "Components.h"
#include "DayReportScene.h"
#include "Entities.h"
#include "GameConfig.h"
#include "HowToPlayScene.h"
#include "MainGameScene.h"
#include "PhysicsContext.h"
#include "RenderContext.h"
#include "SDL3_image/SDL_image.h"
#include "SpriteDims.h"
#include "StartMenuScene.h"
#include "Systems.h"
#include "Utils.h"

#include <SDL3/SDL.h>
#include <iostream>

namespace cafe
{

void CafeGame::init()
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
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
    // Render into a fixed logical canvas; SDL scales it up to the
    // physical window. INTEGER_SCALE keeps pixel art crisp (whole
    // multiples only, letterboxing the remainder).
    SDL_SetRenderLogicalPresentation(renderer,
                                     LOGICAL_W,
                                     LOGICAL_H,
                                     SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);
    SDL_SetWindowResizable(window, true);
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    _icon = IMG_Load("res/icon.png");
    SDL_SetWindowIcon(window, _icon);

    RenderContext::init(window, renderer);

    _window   = window;
    _renderer = renderer;
}

void CafeGame::run()
{
    while (_next != SceneId::Quit)
    {
        std::unique_ptr<Scene> scene = makeScene(_next);
        scene->init(_renderer);
        SceneId next = scene->run();
        scene->cleanup();
        _next = next;
    }
}

std::unique_ptr<Scene> CafeGame::makeScene(SceneId id)
{
    switch (id)
    {
    case SceneId::StartMenu:
        return std::make_unique<StartMenuScene>();
    case SceneId::DayReport:
        return std::make_unique<DayReportScene>();
    case SceneId::HowToPlay:
        return std::make_unique<HowToPlayScene>();
    case SceneId::MainGame:
    default:
        return std::make_unique<MainGameScene>();
    }
}

void CafeGame::destroy()
{
    SDL_DestroyRenderer(_renderer);
    SDL_DestroyWindow(_window);
    SDL_DestroySurface(_icon);
    SDL_Quit();
}

} // namespace cafe
