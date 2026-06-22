#pragma once
#include <string>
#include <unordered_map>
#include "Texture.h"

namespace cafe
{
class SpriteSheet;
class AssetManager final
{
public:
    AssetManager() = default;
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;
    AssetManager(AssetManager&&) = default;
    AssetManager& operator=(AssetManager&&) = default;

    void init(SDL_Renderer* renderer);
    // loads a texture with path (inside res/ dir), if not in map already
    const Texture& getTexture(std::string_view filename);
    void clear() { _textures.clear(); }
private:
    std::unordered_map<std::string, Texture> _textures{};
    SDL_Renderer* _renderer{};

    static constexpr std::string RES_DIR_PATH = "res/";
};
}