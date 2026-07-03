#include "SpawnButtonFactory.h"

#include "Components.h"
#include "PhysicsContext.h"
#include "RenderLayers.h"
#include "SpriteDims.h"
#include "SupplySystem.h"
#include "Texture.h"
#include "AssetManager.h"

namespace cafe
{
namespace
{
b2ShapeId createSlotSensor(PhysicsContext& physics, WorldPos worldPos)
{
    constexpr float CHECK_RADIUS = texToWorldDistance(CUP_DIMS.x/2);

    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_staticBody;
    bodyDef.position = {worldPos.x, worldPos.y};
    b2BodyId bodyId = b2CreateBody(physics.world(), &bodyDef);

    b2ShapeDef shape = b2DefaultShapeDef();
    shape.isSensor = true;
    shape.enableSensorEvents = true;
    shape.filter.categoryBits = filter::DROPSPACE_SENSOR; // just reuse of mask
    shape.filter.maskBits     = filter::CUP_SOLID | filter::DRAGGABLE;
    const b2Circle circle = {{0.f, 0.f}, CHECK_RADIUS};
    return b2CreateCircleShape(bodyId, &shape, &circle);
}
} // namespace

// A button is a plain UI sprite: Transform + Drawable + SpawnButton, with no
// PhysicsBody and no DragIntent, so it can be clicked but never dragged.
bagel::Entity createSpawnButton(AssetManager& assets, PhysicsContext& physics, WorldPos pos, DropType item)
{
    // Cup/pastry drop in from above the screen; ice drops from the ice machine spout.
    const WorldPos spawnPos =
        (item == DropType::Ice)
            ? WorldPos{pos.x, pos.y + texToWorldScale(BUTTON_DIMS.y) + 0.5f} // ice spawns above the button
            : WorldPos{ (item == DropType::Cup) ? supply::CUP_SPAWN_X : supply::PASTRY_SPAWN_X,
                        supply::DROP_FROM_Y };
    b2ShapeId slotSensor = createSlotSensor(physics, spawnPos);

    auto ent = bagel::Entity::create();
    ent.add(
        SpawnButton{ .item = item, .spawnSlotSensor = slotSensor, .spawnPos = spawnPos }
    );

    // ice button is invisible
    if (item != DropType::Ice)
    {
        // TODO: change based on cup vs pastry when sprites ready
        const char*      texPath = "buttons.png";

        const Texture& tex = assets.getTexture(texPath);
        const float srcX = item == DropType::Cup ? 0.f : 0.f + BUTTON_DIMS.x;
        const SDL_FRect src = { srcX, 0.f, BUTTON_DIMS.x, BUTTON_DIMS.y };

        const float halfW = texToWorldScale(BUTTON_DIMS.x);
        const float halfH = texToWorldScale(BUTTON_DIMS.y);

        ent.addAll(
            Transform{ .x = pos.x, .y = pos.y, .w = halfW, .h = halfH },
            Drawable{ tex.get(), src, layer::STATIC_OVERLAY }
        );
    }
    else
    {
        ent.add(Transform{.x = pos.x,
                          .y = pos.y,
                          .w = texToWorldScale(supply::ICE_BUTTON_W_PX),
                          .h = texToWorldScale(supply::ICE_BUTTON_H_PX)});
        // ===================DEBUG========================================
        // auto& debugTex = assets.getTexture("particle.png");// ent.add(Drawable{debugTex.get(), debugTex.getFullSrcRect(), layer::STATIC_OVERLAY });
    }
    return ent;
}
} // namespace cafe
