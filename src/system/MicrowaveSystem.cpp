#include "MicrowaveSystem.h"

#include "AssetManager.h"
#include "Components.h"
#include "Entities.h"      // createPastry, destroyDeliveredItem
#include "RenderLayers.h"
#include "SupplySystem.h"  // supply::MICROWAVE_SPAWN_POS
#include "Transform.h"     // texToWorldScale
#include <bagel.h>
#include <algorithm>       // std::clamp
#include <cmath>           // std::floor
#include <optional>
#include <vector>

namespace cafe
{
namespace
{
// Position of the number display relative to the oven center (world meters, Y-up).
// Edit freely to align with the number-pad face on the oven sprite.
constexpr SDL_FPoint NUMBER_OFFSET = { 1.65f, 0.85f };

bagel::Entity createOvenGlow(AssetManager& assets, bagel::Entity oven)
{
    const SpriteSheet& sheet = assets.getSpriteSheet(OVEN_SPRITE_DATA);
    const auto& ovenT = oven.get<Transform>();

    auto ent = bagel::Entity::create();
    ent.addAll(
        Transform{ .x = ovenT.x, .y = ovenT.y, .w = ovenT.w, .h = ovenT.h },
        Drawable{ assets.getTexture(OVEN_TEX).get(),
                  sheet.getFrameRect(OVEN_GLOW_SPRITE_ID),
                  layer::STATIC_OVERLAY,
                  SDL_Color{ 255, 255, 255, 77 } }, // 30% opacity
        ChildOf(oven /*no offset*/)
    );
    return ent;
}

bagel::Entity createOvenNumber(AssetManager& assets, bagel::Entity oven)
{
    const SpriteSheet& sheet = assets.getSpriteSheet(OVEN_NUMBERS_DATA);
    const SDL_FPoint   sz    = sheet.spriteSize(); // 3 x 5 px
    const float        halfW = texToWorldScale(sz.x);
    const float        halfH = texToWorldScale(sz.y);

    const auto& ovenT = oven.get<Transform>();

    auto ent = bagel::Entity::create();
    ent.addAll(
        Transform{ .x = ovenT.x, .y = ovenT.y, .w = halfW, .h = halfH },
        Drawable{ assets.getTexture(OVEN_NUMBERS_TEX).get(),
                  sheet.getFrameRect(0),
                  layer::STATIC_OVERLAY },
        ChildOf(oven, NUMBER_OFFSET, /*isWorldOffset=*/true)
    );
    return ent;
}
} // namespace

void microwaveSystem(AssetManager& assets, PhysicsContext& physics, float dt)
{
    static const bagel::Mask pastryMask =
        bagel::MaskBuilder().set<DragIntent>().set<Pastry>().build();
    static const bagel::Mask mwMask =
        bagel::MaskBuilder().set<Microwave>().set<Transform>().build();

    // Gather microwaves once (normally just one); reused by cook below.
    std::vector<bagel::Entity> microwaves;
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
        if (e.test(mwMask))
            microwaves.push_back(e);

    // --- Intake: a released pastry dropped onto a microwave ---
    // Defer destruction until after iteration (structural change), like deliverySystem.
    std::optional<bagel::ent_type> absorbed;
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(pastryMask)) continue;

        auto& intent = e.get<DragIntent>();

        if (intent.intentType != DragIntentType::released) continue;

        // Primary: the sensor-driven dropSpaceEntity. Fallback: geometric overlap.
        // The sensor's begin-touch only fires on a held ENTER transition, so an item
        // that started already inside the zone never gets dropSpaceEntity set; the
        // overlap test catches that case without depending on any event.
        bool            onMicrowave = false;
        bagel::ent_type targetId{};
        if (intent.dropSpaceEntity.has_value())
        {
            bagel::Entity t{ *intent.dropSpaceEntity };
            if (t.has<Microwave>())
            {
                targetId    = *intent.dropSpaceEntity;
                onMicrowave = true;
            }
        }
        else if (e.has<Transform>())
        {
            const auto& itemT = e.get<Transform>();
            for (auto mw : microwaves)
                if (boxesOverlap(itemT, mw.get<Transform>()))
                {
                    targetId    = mw.entity();
                    onMicrowave = true;
                    break;
                }
        }
        if (!onMicrowave) continue;

        bagel::Entity target{ targetId };
        auto&         mw = target.get<Microwave>();
        if (mw.busy)
        {
            // Occupied: drop the target so releaseEntity lets the pastry fall normally.
            intent.dropSpaceEntity = std::nullopt;
            continue;
        }

        // Free: capture the type, start heating, remove the pastry from the world.
        mw.busy    = true;
        mw.timer   = 0.f;
        mw.cooking = e.get<Pastry>().type;
        mw.display = createOvenNumber(assets, target);
        mw.glow    = createOvenGlow(assets, target);
        target.get<Drawable>().srcRect =
            assets.getSpriteSheet(OVEN_SPRITE_DATA).getFrameRect(OVEN_COOKING_SPRITE_ID);
        intent.dropSpaceEntity = std::nullopt;
        absorbed = e.entity();
    }
    if (absorbed)
        destroyDeliveredItem(*absorbed);

    // --- Cook + update number + spit-out ---
    const SpriteSheet& numbersSheet = assets.getSpriteSheet(OVEN_NUMBERS_DATA);
    for (auto mw : microwaves)
    {
        auto& m = mw.get<Microwave>();
        if (!m.busy) continue;

        m.timer += dt;

        // Update countdown digit sprite.
        const int digit = std::clamp(static_cast<int>(std::floor(m.heatTime - m.timer)), 0, 5);
        m.display.get<Drawable>().srcRect = numbersSheet.getFrameRect(digit);

        if (m.timer >= m.heatTime)
        {
            // Destroy the number overlay and glow.
            m.display.destroy();
            m.display = bagel::Entity{ bagel::ent_type(-1) };
            m.glow.destroy();
            m.glow = bagel::Entity{ bagel::ent_type(-1) };

            // Restore idle oven sprite.
            mw.get<Drawable>().srcRect =
                assets.getSpriteSheet(OVEN_SPRITE_DATA).getFrameRect(OVEN_DEFAULT_SPRITE_ID);

            // Spit out heated pastry.
            auto pastry = createPastry(assets, physics, supply::MICROWAVE_SPAWN_POS, m.cooking);
            pastry.get<Pastry>().temperature = Temperature::Hot;

            m.busy    = false;
            m.timer   = 0.f;
            m.cooking = PastryType::count;
        }
    }
}
} // namespace cafe
