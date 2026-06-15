#include "LiquidDropFactory.h"
#include "AssetManager.h"
#include "Components.h"
#include "RenderLayers.h"
#include "PhysicsContext.h"
#include "PhysicsFilters.h"
#include "Texture.h"
#include <box2d/box2d.h>

namespace cafe
{
bagel::Entity createLiquidDrop(PhysicsContext& physics, AssetManager& assets, WorldPos pos, Ingredient kind)
{
    static constexpr auto TEX = "particle.png";
    const Texture& tex = assets.getTexture(TEX);

    auto ent = bagel::Entity::create();
    constexpr float r = 0.06f; // ~0.5 px radius. Raise to 0.10f if drops jitter.

    b2BodyDef bd = b2DefaultBodyDef();
    bd.type     = b2_dynamicBody;
    bd.fixedRotation = false;
    bd.position = { pos.x, pos.y };
    bd.linearDamping = 4.f;
    bd.isBullet = true; // CCD — prevents tunneling through the thin cup walls
    bd.userData = reinterpret_cast<void*>(static_cast<uintptr_t>(ent.entity().id));
    b2BodyId body = b2CreateBody(physics.world(), &bd);

    // Visitor side of a sensor pair must also opt in to sensor events.
    b2ShapeDef sd = b2DefaultShapeDef();
    sd.density             = 8.f;
    sd.material.friction   = 2.f;
    sd.enableSensorEvents  = true;
    sd.filter.categoryBits = filter::LIQUID;
    sd.filter.maskBits     = filter::MASK_LIQUID | filter::LIQUID;

    b2Circle circle{ {0.f, 0.f}, r };
    b2CreateCircleShape(body, &sd, &circle);

    ent.addAll(
        Liquid{ kind },
        Transform{ .x = pos.x, .y = pos.y, .w = r, .h = r },
        Drawable{ tex.get(),tex.getFullSrcRect() , layer::LIQUID },
        PhysicsBody{ body }
    );
    return ent;
}

} // namespace cafe
