#include "IceMachineFactory.h"
#include "AssetManager.h"
#include "Components.h"
#include "PhysicsContext.h"
#include "PhysicsFilters.h"
#include "RenderLayers.h"
#include "Texture.h"
#include "SpawnButtonFactory.h"
#include <box2d/box2d.h>

namespace cafe
{
namespace
{
constexpr float COLLIDER_EPSILON = 0.05f; // shrink so the solid body sits just inside the sprite edges
}

bagel::Entity createIceMachine(AssetManager& assets, PhysicsContext& physics, WorldPos pos)
{
    static constexpr auto TEX = "ice_machine.png";
    const Texture& tex = assets.getTexture(TEX);

    float halfW = texToWorldScale(tex.getSize().x); // world half-extents — tune freely
    float halfH = texToWorldScale(tex.getSize().y);

    auto ent = bagel::Entity::create();

    // Static solid body, slightly smaller than the sprite so it doesn't visually clip neighbors.
    b2BodyDef bd = b2DefaultBodyDef();
    bd.type      = b2_staticBody;
    bd.position  = { pos.x, pos.y };
    bd.userData  = reinterpret_cast<void*>(static_cast<uintptr_t>(ent.entity().id));
    b2BodyId body = b2CreateBody(physics.world(), &bd);

    b2ShapeDef machineShape = b2DefaultShapeDef();
    machineShape.filter.categoryBits = filter::FURNITURE;
    machineShape.filter.maskBits     = filter::MASK_FURNITURE;
    const b2Polygon collider = b2MakeOffsetBox(halfW - COLLIDER_EPSILON, halfH - COLLIDER_EPSILON,
                                                { 0.f, 0.f }, b2Rot_identity);
    b2CreatePolygonShape(body, &machineShape, &collider);

    ent.addAll(
        Transform{ .x = pos.x, .y = pos.y, .w = halfW, .h = halfH },
        Drawable{ tex.get(), tex.getFullSrcRect(), layer::STATIC_ON_BARTOP },
        PhysicsBody{ body },
        IceMachine{}
    );

    // the button on the machine. reusing this factory
    auto spawnButtonEnt = createSpawnButton(assets, physics, pos, DropType::Ice);
    // make button child of the machine
    spawnButtonEnt.add(
        ChildOf(ent, {-0.3f, -0.87f}, true)
    );

    return ent;
}
} // namespace cafe
