#include "CoffeeMachineFactory.h"
#include "AssetManager.h"
#include "Components.h"
#include "RenderLayers.h"
#include "PhysicsContext.h"
#include "Texture.h"

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
    {  1.3f, -0.3f }, // Milk
    {  0.13f, 0.0f }, // Water
};

constexpr SDL_FPoint PIXEL_BUTTON_OFFSET[INGREDIENT_COUNT] = {
    {  0.0f, 0.0f },
    {  0.0f, 0.0f },
    {  0.0f, 0.0f },
};

bagel::Entity createPipe(WorldPos machinePos, bagel::Entity& machineEnt, Ingredient kind)
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
} // namespace

bagel::Entity createCoffeeMachine(PhysicsContext& physics, AssetManager& assets, WorldPos pos)
{
    const Texture& tex = assets.getTexture(TEX);
    auto [x, y] = tex.getSize();
    auto ent = bagel::Entity::create();

    const float halfW = screenToWorldScale(x);
    const float halfH = screenToWorldScale(y);
    Transform t{ .x = pos.x, .y = pos.y, .w = halfW, .h = halfH };

    // Kinematic body, no fixtures — the machine is just a position anchor.
    b2BodyDef bd = b2DefaultBodyDef();
    bd.type     = b2_kinematicBody;
    bd.position = { t.x, t.y };
    bd.userData = reinterpret_cast<void*>(static_cast<uintptr_t>(ent.entity().id));
    b2BodyId body = b2CreateBody(physics.world(), &bd);

    ent.addAll(
        t,
        Drawable{ tex.get(), tex.getFullSrcRect(), layer::STATIC_ON_BARTOP },
        PhysicsBody{ body }
    );

    createPipe(pos, ent,Ingredient::Coffee);
    createPipe(pos, ent,Ingredient::Milk);
    createPipe(pos, ent,Ingredient::Water);

    return ent;
};
}