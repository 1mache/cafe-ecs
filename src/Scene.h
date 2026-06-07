#pragma once
#include "AssetManager.h"

namespace cafe
{
class Scene final
{
public:
    explicit Scene(std::string_view bgTexturePath, SDL_Renderer* renderer)
    : _bgTexturePath(bgTexturePath), _renderer(renderer)
    {
        // preload the bg texture of the scene
        _assetManager.getTexture(bgTexturePath, renderer);
    }
    AssetManager& getAssetManager() { return _assetManager; }
    const Texture& getBgTexture() { return _assetManager.getTexture(_bgTexturePath, _renderer);}
private:
    const std::string _bgTexturePath;
    SDL_Renderer*     _renderer;
    AssetManager      _assetManager;
};
} //namespace cafe