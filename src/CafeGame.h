#pragma once
#include "AssetManager.h"
#include "GameConfig.h"
#include "MainGameScene.h"
#include "Scene.h"
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
    std::unique_ptr<Scene> _currentScene{std::make_unique<MainGameScene>()};
    SDL_Renderer* _renderer{};
    SDL_Window*   _window{};

    // same as through scene, just shortcut
    AssetManager& getAssetManager()
    {
        return _currentScene->getAssetManager();
    }
};
} // namespace cafe
