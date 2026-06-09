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
    void destroy() const;

private:
    Scene         _currentScene{INIT_SCENE_BG};
    SDL_Renderer* _renderer{};
    SDL_Window*   _window{};

    // same as through scene, just shortcut
    AssetManager& getAssetManager()
    {
        return _currentScene.getAssetManager();
    }

    static constexpr int CUP_CAPACITY = 50;

    static constexpr auto INIT_SCENE_BG = "bg.png";
};
} // namespace cafe
