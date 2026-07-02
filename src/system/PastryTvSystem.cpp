#include "PastryTvSystem.h"

#include "AssetManager.h"
#include "Components.h"
#include "SpriteSheet.h"
#include <bagel.h>

namespace cafe
{
namespace
{
constexpr auto PROPS_TEX  = "props.png";
constexpr auto PROPS_DATA = "props.json";
} // namespace

void pastryTvSystem(AssetManager& assets, float dt)
{
    static const bagel::Mask mask =
        bagel::MaskBuilder().set<PastryTv>().set<Drawable>().build();

    const SpriteSheet& props      = assets.getSpriteSheet(PROPS_TEX, PROPS_DATA);
    const int          pastryFrom = props.getTagBounds("pastry").first;

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask)) continue;

        auto& tv = e.get<PastryTv>();
        tv.timer += dt;
        if (tv.timer < PASTRY_TV_ROTATE_TIME) continue;

        tv.timer -= PASTRY_TV_ROTATE_TIME;
        tv.index = (tv.index + 1) % static_cast<int>(PastryType::count);
        e.get<Drawable>().srcRect = props.getFrameRect(pastryFrom + tv.index);
    }
}

PastryType currentTvPastryType()
{
    static const bagel::Mask mask = bagel::MaskBuilder().set<PastryTv>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask)) continue;
        return static_cast<PastryType>(e.get<PastryTv>().index);
    }
    return PastryType::count; // no TV -> createPastry picks a random type
}
} // namespace cafe
