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
static constexpr auto TEX = "machine.png";

// Pour-pipe positions relative to the machine center (world meters, Y-up).
constexpr WorldPos PIPE_OFFSET[INGREDIENT_COUNT] = {
    { -0.4f, -0.5f }, // Coffee
    {  0.0f, -0.5f }, // Milk
    {  0.4f, -0.5f }, // Water
};

bagel::Entity createPipe(WorldPos machinePos, Ingredient kind)
{
    const WorldPos off = PIPE_OFFSET[static_cast<size_t>(kind)];
    auto ent = bagel::Entity::create();
    ent.addAll(
        Transform{ .x = machinePos.x + off.x, .y = machinePos.y + off.y, .w = 0.f, .h = 0.f },
        LiquidSpawner{ .kind = kind, .interval = 0.05f, .accumulator = 0.f,
                       .active = false, .offset = {} },
        PourIntent{}
    );
    return ent;
}
} // namespace

CoffeeMachine createCoffeeMachine(AssetManager& assets, WorldPos pos)
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
    b2BodyId body = b2CreateBody(PhysicsContext::world(), &bd);

    ent.addAll(
        t,
        Drawable{ tex.get(), tex.getFullSrcRect(), layer::STATIC_ON_BARTOP },
        PhysicsBody{ body }
    );

    return CoffeeMachine{
        ent,
        {
            createPipe(pos, Ingredient::Coffee),
            createPipe(pos, Ingredient::Milk),
            createPipe(pos, Ingredient::Water),
        }
    };
}
} // namespace cafe