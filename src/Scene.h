#pragma once
#include "AssetManager.h"

namespace cafe
{
class Scene final
{
public:
    explicit Scene(std::string_view bgTexturePath)
    : _bgTexturePath(bgTexturePath)
    {}
    void init(SDL_Renderer* renderer);
    AssetManager& getAssetManager() { return _assetManager; }
    const Texture& getBgTexture() { return _assetManager.getTexture(_bgTexturePath);}
private:
    const std::string _bgTexturePath;
    SDL_Renderer*     _renderer{};
    AssetManager      _assetManager;
};
} //namespace cafe