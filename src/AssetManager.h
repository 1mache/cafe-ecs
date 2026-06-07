#pragma once
#include <string>
#include <unordered_map>
#include "Texture.h"

namespace cafe
{
class AssetManager final
{
public:
    // loads a texture with path (inside res/ dir), if not in map already
    // TODO: take renderer in ctor
    const Texture& getTexture(std::string_view path, SDL_Renderer* renderer);
    void clear() { _textures.clear(); }
private:
    std::unordered_map<std::string, Texture> _textures{};

    static constexpr std::string RES_DIR_PATH = "res/";
};
}