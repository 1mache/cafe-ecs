#pragma once
#include "AssetManager.h"

namespace cafe
{
class Scene
{
public:
    Scene() = default;

    // template method pattern
    void init(SDL_Renderer* renderer);
    void run();
    void cleanup();

    AssetManager& getAssetManager() { return _assetManager; }
    SDL_Renderer* getRenderer() const { return _renderer; }

    virtual ~Scene() = default;
    Scene(const Scene&)            = delete;
    Scene& operator=(const Scene&) = delete;
protected:
    virtual void onInit()              = 0;
    virtual bool onUpdate(float dt)    = 0;
    virtual void onCleanup()           = 0;
private:
    SDL_Renderer*     _renderer{};
    AssetManager      _assetManager;
};
} //namespace cafe