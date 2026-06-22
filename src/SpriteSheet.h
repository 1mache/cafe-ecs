#pragma once
#include "SDL3/SDL_rect.h"

#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace cafe
{
class SpriteSheet
{
public:
    explicit SpriteSheet(std::string textureFilename)
    : _textureFilename(std::move(textureFilename))
    {}

    void load(std::string_view filename);

    int frameCount() const
    {
        return static_cast<int>(_frames.size());
    }

    [[nodiscard]]
    const std::string& textureFilename() const { return _textureFilename; }

    SDL_FRect getFrame(int frameIndex) const;
    std::pair<int, int> getTagBounds(const std::string& tag) const;

private:
    std::unordered_map<std::string, std::pair<int,int>> _tagMap{};
    std::vector<SDL_FRect>                              _frames{};
    std::string                                         _textureFilename{};
    SDL_FPoint                                          _spriteSize{};
};
}