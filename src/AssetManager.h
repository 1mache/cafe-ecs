#pragma once
#include <string>
#include <unordered_map>
#include "Texture.h"
#include "SpriteSheet.h"

namespace cafe
{
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
    // loads a texture for this
    const SpriteSheet& getSpriteSheet(std::string_view textureFilename,
                                      std::string_view spriteSheetFilename);
    // for already loaded sprite sheets
    const SpriteSheet& getSpriteSheet(std::string_view spriteSheetFilename);

    void clear()
    {
        _textures.clear();
        _spriteSheets.clear();
    }
private:
    std::unordered_map<std::string, Texture>      _textures{};
    std::unordered_map<std::string, SpriteSheet>  _spriteSheets{};
    SDL_Renderer* _renderer{};

    static constexpr auto RES_DIR_PATH = "res/";
};
}