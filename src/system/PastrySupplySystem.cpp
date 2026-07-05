#include "PastrySupplySystem.h"

#include "AssetManager.h"
#include "Components.h"
#include "Entities.h" // createPastry
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
        e.get<Drawable>().srcRect =
            props.getFrameRect(pastryFrom + static_cast<int>(tv.queue[tv.index]));
    }
}

// The pastry-TV icon entity is itself the spawn button (createPastryTv builds it
// via createSpawnButton), so PastryTv and Button live on one entity: a single
// scan reads the shown pastry index and the click, then spawns after the loop.
void pastrySupplySystem(AssetManager& assets, PhysicsContext& physics)
{
    static const bagel::Mask mask =
        bagel::MaskBuilder().set<PastryTv>().set<Button>().build();

    bool       shouldSpawn{};
    WorldPos   spawnPos{};
    PastryType type{ PastryType::count };

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask)) continue;

        auto& b = e.get<Button>();
        if (b.kind != ButtonKind::Spawn) continue;
        if (!consumeSpawnRequest(b)) continue;

        shouldSpawn   = true;
        spawnPos      = b.spawnPos;
        const auto& tv = e.get<PastryTv>();
        type          = tv.queue[tv.index];
    }

    if (shouldSpawn) createPastry(assets, physics, spawnPos, type);
}
} // namespace cafe
