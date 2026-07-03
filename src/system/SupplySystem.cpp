#include "SupplySystem.h"

#include "Components.h"
#include "Entities.h"
#include "Transform.h"
#include "Utils.h"

#include <bagel.h>
#include <vector>

namespace cafe
{
void supplyButtonSystem(AssetManager& assets, PhysicsContext& physics)
{
    static const bagel::Mask buttonMask = bagel::MaskBuilder().set<SpawnButton>().build();

    std::vector<std::pair<WorldPos,DropType>> toSpawn;
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(buttonMask)) continue;

        auto& b = e.get<SpawnButton>();
        // Pastry has its own pastrySupplySystem; don't consume its click here.
        if (b.item == DropType::Pastry) continue;
        if (!consumeSpawnRequest(b)) continue;

        toSpawn.emplace_back(b.spawnPos, b.item);
    }

    for (auto& sp : toSpawn)
    {
        if (sp.second == DropType::Cup)
            createCup(assets, physics, sp.first);
        else if (sp.second == DropType::Ice)
            createIceCube(assets, physics, sp.first);
        // Pastry is handled separately by pastrySupplySystem (its button is the TV icon).
    }
}
} // namespace cafe
