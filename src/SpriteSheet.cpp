#include "SpriteSheet.h"

#include "Utils.h"

#include <nlohmann/json.hpp>
#include <fstream>

namespace cafe
{
void SpriteSheet::load(std::string_view filename)
{
    using json = nlohmann::json;

    std::ifstream ifs(filename.data());

    assertFatal(ifs.is_open(),
        "Could not open file: " + std::string(filename));

    json spriteSheet = json::parse(ifs);
    try
    {
        std::vector<json> frameData;
        spriteSheet.at("frames").get_to(frameData);
        assertFatal(frameData.size() > 0,
            "No frames in sprite sheet" + std::string(filename));

        json sourceSize = frameData[0].at("sourceSize");
        _spriteSize = {sourceSize.at("w").get<float>(), sourceSize.at("h").get<float>()};

        // get all src rects from json
        std::ranges::transform(frameData, std::back_inserter(_frames),
            [](json& frame)
            {
                // name of field that has the rect is frame. confusing
                json rect = frame.at("frame");
                return SDL_FRect{
                    (rect.at("x").get<float>()),
                    (rect.at("y").get<float>()),
                    (rect.at("w").get<float>()),
                    (rect.at("h").get<float>())
                };
            });

        for (auto& tag : spriteSheet.at("meta").at("frameTags"))
        {
            _tagMap[tag.at("name").get<std::string>()] = {
                tag.at("from").get<int>(),
                tag.at("to").get<int>()
            };
        }
    } catch (const nlohmann::json::exception& e)
    {
        ifs.close();
        fatalError(e.what());
    }


    ifs.close();
}
SDL_FRect SpriteSheet::getFrame(int frameIndex) const
{
    if (frameIndex < 0 || frameIndex >= frameCount())
    {
        fatalError(("SpriteSheet::getFrame got Invalid frame index" +
                    std::to_string(frameIndex))
                       .data());
    }

    return _frames[static_cast<std::size_t>(frameIndex)];
}
std::pair<int, int> SpriteSheet::getTagBounds(const std::string& tag) const
{
    const auto it = _tagMap.find(tag);
    if (it == _tagMap.end())
        fatalError(("SpriteSheet::getTagBounds got Invalid Tag name" + tag).data());

    return it->second;
}
} // namespace cafe