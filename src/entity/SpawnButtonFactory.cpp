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
    constexpr float CHECK_RADIUS = screenToWorldDistance(CUP_DIMS.x / 2);

    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_staticBody;
    bodyDef.position = {worldPos.x, worldPos.y};
    b2BodyId bodyId = b2CreateBody(physics.world(), &bodyDef);

    b2ShapeDef shape = b2DefaultShapeDef();
    shape.isSensor = true;
    shape.enableSensorEvents = true;
    shape.filter.categoryBits |= filter::DROPSPACE_SENSOR;
    shape.filter.maskBits |= filter::CUP_SOLID | filter::DRAGGABLE;
    const b2Circle circle = {bodyDef.position, CHECK_RADIUS};
    return b2CreateCircleShape(bodyId, &shape, &circle);
}
} // namespace

// A button is a plain UI sprite: Transform + Drawable + SpawnButton, with no
// PhysicsBody and no DragIntent, so it can be clicked but never dragged.
bagel::Entity createSpawnButton(AssetManager& assets, PhysicsContext& physics, WorldPos pos, DropType item)
{
    const char*      texPath = (item == DropType::Cup) ? "cup.png" : "props.png";
    const SDL_FPoint dims    = (item == DropType::Cup) ? CUP_DIMS : PROP_DIMS;

    const Texture& tex = assets.getTexture(texPath);
    const SDL_FRect src = { 0.f, 0.f, dims.x, dims.y };

    const float halfW = screenToWorldScale(dims.x);
    const float halfH = screenToWorldScale(dims.y);

    const float     spawnX     = (item == DropType::Cup) ? supply::CUP_SPAWN_X : supply::PASTRY_SPAWN_X;
    const WorldPos  spawnPos   = { spawnX, supply::DROP_FROM_Y };
    const b2ShapeId slotSensor = createSlotSensor(physics, spawnPos);

    auto ent = bagel::Entity::create();
    ent.addAll(
        Transform{ .x = pos.x, .y = pos.y, .w = halfW, .h = halfH },
        Drawable{ tex.get(), src, layer::UI1 },
        SpawnButton{ .item = item, .slotSensor = slotSensor, .spawnPos = spawnPos }
    );
    return ent;
}
} // namespace cafe
