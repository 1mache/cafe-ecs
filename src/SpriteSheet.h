#pragma once
#include "SDL3/SDL_rect.h"

#include <ranges>
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
    [[nodiscard]]
    SDL_FRect getFrameRect(int frameIndex) const;
    [[nodiscard]]
    auto tags() const { return std::views::keys(_tagMap); }
    [[nodiscard]]
    std::pair<int, int> getTagBounds(const std::string& tag) const;
    [[nodiscard]]
    int tagFrameCount(const std::string& tag) const
    {
        auto [from, to] = getTagBounds(tag); // fatalErrors if tag missing
        return to - from + 1;                // bounds are inclusive
    }

private:
    std::unordered_map<std::string, std::pair<int,int>> _tagMap{};
    std::vector<SDL_FRect>                              _frames{};
    std::string                                         _textureFilename{};
    SDL_FPoint                                          _spriteSize{};
};
}