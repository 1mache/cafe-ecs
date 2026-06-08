#include "AssetManager.h"

#include "Utils.h"

#include <memory>

namespace cafe
{

Texture& AssetManager::getTexture(std::string_view path, SDL_Renderer* renderer)
{
    std::string fullPath = RES_DIR_PATH + std::string(path);
    auto [it, success] = _textures.try_emplace(fullPath);
    if (success) // key didn't previously exist
    {
        bool loaded = it->second.loadFromFile(fullPath, renderer);
        assertFatal(loaded, "Error loading " + fullPath + ": " + SDL_GetError());
    }

    return it->second;
}
} // namespace cafe