#pragma once
#include "AssetManager.h"
#include "Scene.h"
#include "GameConfig.h"

namespace cafe
{
class CafeGame final
{
public:
    void init();
    void run();
    void destroy();

    // same as through scene, just shortcut
    AssetManager& getAssetManager()
    {
        return _currentScene.getAssetManager();
    }

private:
    Scene         _currentScene{INIT_SCENE_BG, _renderer};
    SDL_Renderer* _renderer{};
    SDL_Window*   _window{};

    static constexpr auto INIT_SCENE_BG = "bg.png";
};
} // namespace cafe
