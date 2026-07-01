#include "AssetManager.h"

#include "Utils.h"

#include <memory>

namespace cafe
{

void AssetManager::init(SDL_Renderer* renderer)
{
    _renderer = renderer;
}

const Texture& AssetManager::getTexture(std::string_view filename)
{
    std::string fullPath = std::string(RES_DIR_PATH) + std::string(filename);
    auto [it, success] = _textures.try_emplace(filename.data());
    if (success) // key didnt previously exist
    {
        bool loaded = it->second.loadFromFile(fullPath, _renderer);
        assertFatal(loaded, "Error loading " + fullPath + ": "
                                                        + SDL_GetError());
    }

    return it->second;
}
const SpriteSheet& AssetManager::getSpriteSheet(std::string_view textureFilename,
                                                std::string_view spriteSheetFilename)
{
    auto& texture = getTexture(textureFilename);
    auto [it, success] = _spriteSheets.try_emplace(spriteSheetFilename.data(), std::string(textureFilename));
    if (success)
    {
        it->second.load(std::string(RES_DIR_PATH) + std::string(spriteSheetFilename));
    }

    return it->second;
}
const SpriteSheet& AssetManager::getSpriteSheet(std::string_view spriteSheetFilename)
{
    auto it = _spriteSheets.find(spriteSheetFilename.data());
    if (it == _spriteSheets.end()) fatalError("Trying to access spritesheet " + std::string(spriteSheetFilename) + " that doesnt exist");

    return it->second;
}
} // namespace cafe