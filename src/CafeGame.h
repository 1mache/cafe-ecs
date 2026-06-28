#pragma once
#include "GameConfig.h"
#include "Scene.h"
#include "SceneId.h"
#include <SDL3/SDL.h>
#include <memory>

namespace cafe
{
class CafeGame final
{
public:
    void init();
    void run();
    void destroy();

private:
    std::unique_ptr<Scene> makeScene(SceneId id);

    SDL_Renderer* _renderer{};
    SDL_Window*   _window{};
    SceneId       _next{ SceneId::MainGame };
};
} // namespace cafe
