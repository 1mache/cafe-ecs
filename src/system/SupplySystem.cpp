#include "SupplySystem.h"

#include "Components.h"
#include "Entities.h"
#include "PastryTvSystem.h" // currentTvPastryType
#include "Transform.h"
#include "Utils.h"

#include <bagel.h>
#include <box2d/box2d.h>
#include <cstddef>
#include <iostream>
#include <iterator>
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
        if (!b.justPressed) continue;

        // check if something else is currently at spawn slot
        // to not spawn objects inside each other
        b2ShapeId overlaps[1];
        if (b2Shape_GetSensorOverlaps(b.spawnSlotSensor, overlaps, 1) > 0)
        {
            b.justPressed = false; // ignore this spawn request
            continue;
        }

        b.justPressed = false;
        toSpawn.emplace_back(b.spawnPos, b.item);
    }

    for (auto& sp : toSpawn)
    {
        if (sp.second == DropType::Cup)
            createCup(assets, physics, sp.first);
        else if (sp.second == DropType::Pastry)
            createPastry(assets, physics, sp.first, currentTvPastryType());
        else if (sp.second == DropType::Ice)
            createIceCube(assets, physics, sp.first);
    }
}
} // namespace cafe
