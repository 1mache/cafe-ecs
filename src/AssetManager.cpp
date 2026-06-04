#include "AssetManager.h"

#include "Utils.h"

#include <memory>

namespace cafe
{

Texture& AssetManager::getTexture(std::string_view path, SDL_Renderer* renderer)
{
    std::string fullPath = RES_DIR_PATH + std::string(path);
    auto [it, success] = _textures.try_emplace(path.data());
    if (success) // key didnt previously exist
    {
        bool loaded = it->second.loadFromFile(path.data(), renderer);
        assertFatal(loaded, "Error loading " + fullPath + ": "
                                                        + SDL_GetError());
    }

    return it->second;
}
} // namespace cafe