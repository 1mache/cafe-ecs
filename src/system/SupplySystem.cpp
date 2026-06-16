#include "SupplySystem.h"

#include "Components.h"
#include "Entities.h"
#include "SpawnSensor.h"
#include "Transform.h"
#include <bagel.h>
#include <box2d/box2d.h>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <vector>

namespace cafe
{
void supplyButtonSystem(PhysicsContext& physics, AssetManager& assets)
{
    static const bagel::Mask buttonMask = bagel::MaskBuilder().set<SpawnButton>().build();

    // Consume click flags during the scan (field mutation is safe); defer the
    // actual spawns out of the loop since spawning creates entities.
    std::vector<std::pair<WorldPos,DropType>> toSpawn;
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(buttonMask)) continue;

        auto& b = e.get<SpawnButton>();
        if (!b.justPressed) continue;
        if (b2Shape_GetSensorCapacity(b.slotSensor) > 0)
        {
            std::cout << "CANT SPAWN" << std::endl;
            continue;
        }
        b.justPressed = false;
        toSpawn.emplace_back(b.spawnPos, b.item);
    }

    for (auto& sp : toSpawn)
    {
        if (sp.second == DropType::Cup)
            createCup(physics, assets, sp.first);
        else if (sp.second == DropType::Pastry)
            createPastry(physics, sp.first, assets);
    }
}
} // namespace cafe
