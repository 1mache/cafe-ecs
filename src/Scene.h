#pragma once
#include "AssetManager.h"

namespace cafe
{
class Scene
{
public:
    explicit Scene(std::string_view bgTexturePath)
    : _bgTexturePath(bgTexturePath)
    {}

    // template method pattern
    void init(SDL_Renderer* renderer);
    void run();
    void cleanup();

    AssetManager& getAssetManager() { return _assetManager; }
    const Texture& getBgTexture() { return _assetManager.getTexture(_bgTexturePath);}

    virtual ~Scene() = default;
protected:
    virtual void onInit()    = 0;
    virtual void onRun()     = 0;
    virtual void onCleanup() = 0;
private:
    const std::string _bgTexturePath;
    SDL_Renderer*     _renderer{};
    AssetManager      _assetManager;
};
} //namespace cafe