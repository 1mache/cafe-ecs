#pragma once
#include "AssetManager.h"
#include "GameConfig.h"
#include "Scene.h"
#include "bagel.h"

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

    bagel::Entity _machineEnt{static_cast<bagel::ent_type>(-1)};

    // same as through scene, just shortcut
    AssetManager& getAssetManager()
    {
        return _currentScene.getAssetManager();
    }

    static constexpr int CUP_CAPACITY = 50;

    static constexpr auto INIT_SCENE_BG = "bg.png";
};
} // namespace cafe
