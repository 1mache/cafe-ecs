#include "MicrowaveSystem.h"

#include "Components.h"
#include "Entities.h"      // createPastry, destroyDeliveredItem
#include "PhysicsContext.h"
#include "SupplySystem.h"  // supply::MICROWAVE_SPAWN
#include <bagel.h>
#include <vector>

namespace cafe
{
void microwaveSystem(AssetManager& assets, PhysicsContext& physics, float dt)
{
    static const bagel::Mask patMask =
        bagel::MaskBuilder().set<DragIntent>().set<Pastry>().build();
    static const bagel::Mask mwMask =
        bagel::MaskBuilder().set<Microwave>().build();

    // --- Intake: a released pat dropped onto a microwave ---
    // Defer destruction until after iteration (structural change), like deliverySystem.
    std::vector<bagel::ent_type> absorbed;
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(patMask)) continue;

        auto& intent = e.get<DragIntent>();
        if (intent.intentType != DragIntentType::released) continue;
        if (!intent.dropSpaceEntity.has_value()) continue;

        bagel::Entity target{ *intent.dropSpaceEntity };
        if (!target.has<Microwave>()) continue;

        auto& mw = target.get<Microwave>();
        if (mw.busy)
        {
            // Occupied: drop the target so releaseEntity lets the pat fall normally.
            intent.dropSpaceEntity = std::nullopt;
            continue;
        }

        // Free: capture the type, start heating, remove the pat from the world.
        mw.busy    = true;
        mw.timer   = 0.f;
        mw.cooking = e.get<Pastry>().type;
        intent.dropSpaceEntity = std::nullopt;
        absorbed.push_back(e.entity());
    }
    for (auto id : absorbed)
        destroyDeliveredItem(id);

    // --- Cook + spit-out ---
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mwMask)) continue;

        auto& mw = e.get<Microwave>();
        if (!mw.busy) continue;

        mw.timer += dt;
        if (mw.timer >= HEAT_TIME)
        {
            auto pat = createPastry(assets, physics, supply::MICROWAVE_SPAWN, mw.cooking);
            pat.get<Pastry>().temperature = HEATED_TEMPERATURE;

            mw.busy    = false;
            mw.timer   = 0.f;
            mw.cooking = PastryType::count;
        }
    }
}
} // namespace cafe
