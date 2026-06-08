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

    static constexpr auto TEX_COUNTER  = "counter.png";
    static constexpr auto TEX_CUSTOMER = "def_customer.png";
    static constexpr auto TEX_CUP      = "big_cup.png";
    static constexpr auto TEX_PROPS    = "props.png";
    static constexpr auto TEX_BUBBLE   = "bubble.png";
    static constexpr auto TEX_MACHINE  = "machine.png";
    static constexpr auto TEX_CUP_ITEM = "cup.png";
    static constexpr auto TEX_PARTICLE = "particle.png";

    static constexpr auto INIT_SCENE_BG = "bg.png";
};
} // namespace cafe
