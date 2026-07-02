#include "CoffeeMachineFactory.h"
#include "AssetManager.h"
#include "Components.h"
#include "PhysicsContext.h"
#include "RenderLayers.h"
#include "SpriteDims.h"
#include "Texture.h"

#include "PhysicsFilters.h"
#include <box2d/box2d.h>

namespace cafe
{
namespace
{
constexpr auto TEX        = "machine.png";
constexpr auto BUTTON_TEX = "buttons.png";

// Pour-pipe positions relative to the machine center (world meters, Y-up).
constexpr WorldPos PIPE_OFFSET[INGREDIENT_COUNT] = {
    { -1.05f, 0.23f }, // Coffee
    {  0.13f, 0.0f },  // Water
    {  1.3f, -0.3f },  // Milk
};

constexpr int BUTTON_ON_TEX_ID[INGREDIENT_COUNT] = {
    0, 2, 1
};

constexpr WorldPos BUTTON_OFFSET[INGREDIENT_COUNT] = {
    {  -1.05f, 1.2f }, // Coffee
    {  0.10f, 1.2f },  // Water
    {  1.08f, 1.2f },  // Milk
};

bagel::Entity createPipe(WorldPos machinePos, bagel::Entity& machineEnt, LiquidIngredient kind)
{
    const WorldPos off = PIPE_OFFSET[static_cast<size_t>(kind)];
    auto ent = bagel::Entity::create();
    ent.addAll(
        Transform{ .x = machinePos.x, .y = machinePos.y, .w = 0.f, .h = 0.f },
        LiquidSpawner{ .kind = kind, .interval = 0.05f, .accumulator = 0.f,
                       .active = false, .offset = {} },
        ChildOf(machineEnt, {off.x, off.y}, true)
    );
    return ent;
}

bagel::Entity createButton(AssetManager& assets ,WorldPos machinePos, bagel::Entity& machineEnt, LiquidIngredient kind)
{
    auto& tex = assets.getTexture(BUTTON_TEX);
    const auto buttonTexId = static_cast<float>(BUTTON_ON_TEX_ID[static_cast<size_t>(kind)]);
    SDL_FRect srcRect{ .x = buttonTexId * BUTTON_DIMS.x, .y = 0, .w = BUTTON_DIMS.x, .h = BUTTON_DIMS.y };
    const WorldPos off = BUTTON_OFFSET[static_cast<size_t>(kind)];
    auto ent = bagel::Entity::create();
    ent.addAll(
        Drawable{ tex.get(), srcRect, layer::STATIC_OVERLAY},
        Transform{ .x = machinePos.x, .y = machinePos.y,
                   .w = texToWorldScale(BUTTON_DIMS.x),
                   .h = texToWorldScale(BUTTON_DIMS.y) },
        MachineButton{ kind},
        ChildOf(machineEnt, {off.x, off.y}, true)
    );
    return ent;
}
} // namespace

bagel::Entity createCoffeeMachine(AssetManager& assets, PhysicsContext& physics, WorldPos pos)
{
    const Texture& tex = assets.getTexture(TEX);
    auto [x, y] = tex.getSize();
    auto ent = bagel::Entity::create();

    const float halfW = texToWorldScale(x);
    const float halfH = texToWorldScale(y);
    Transform t{ .x = pos.x, .y = pos.y, .w = halfW, .h = halfH };

    // Kinematic body, no fixtures — the machine is just a position anchor.
    b2BodyDef bd = b2DefaultBodyDef();
    bd.type     = b2_kinematicBody;
    bd.position = { t.x, t.y };
    bd.userData = reinterpret_cast<void*>(static_cast<uintptr_t>(ent.entity().id));
    b2BodyId body = b2CreateBody(physics.world(), &bd);

    // Solid collider: top of texture down to the lowest pipe (Milk, y = -0.3).
    constexpr float colliderBottom = PIPE_OFFSET[static_cast<size_t>(LiquidIngredient::Milk)].y + 0.5f;
    const float colliderHalfH   = (halfH - colliderBottom) * 0.5f;
    const float colliderOffsetY = (halfH + colliderBottom) * 0.5f;
    const b2Polygon collider =
        b2MakeOffsetBox(halfW, colliderHalfH, { 0.f, colliderOffsetY }, b2Rot_identity);

    b2ShapeDef machineShape = b2DefaultShapeDef();
    machineShape.filter.categoryBits = filter::MACHINES;
    machineShape.filter.maskBits     = filter::MASK_MACHINES;
    b2CreatePolygonShape(body, &machineShape, &collider);

    ent.addAll(
        t,
        Drawable{ tex.get(), tex.getFullSrcRect(), layer::STATIC_ON_BARTOP },
        PhysicsBody{ body }
    );

    createPipe(pos, ent,LiquidIngredient::Coffee);
    createPipe(pos, ent,LiquidIngredient::Milk);
    createPipe(pos, ent,LiquidIngredient::Water);

    createButton(assets, pos, ent, LiquidIngredient::Coffee);
    createButton(assets, pos, ent, LiquidIngredient::Milk);
    createButton(assets, pos, ent, LiquidIngredient::Water);

    return ent;
};
}