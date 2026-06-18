#include "IceCubeFactory.h"
#include "AssetManager.h"
#include "Components.h"
#include "RenderLayers.h"
#include "PhysicsContext.h"
#include "PhysicsFilters.h"
#include "Texture.h"
#include <box2d/box2d.h>

namespace cafe
{
bagel::Entity createIceCube(AssetManager& assets, PhysicsContext& physics, WorldPos pos)
{
    static constexpr auto TEX = "particle.png";
    const Texture& tex = assets.getTexture(TEX);

    auto ent = bagel::Entity::create();
    constexpr float r = 0.18f; // ~3x a liquid drop (drop r = 0.06). Tune to taste.

    b2BodyDef bd = b2DefaultBodyDef();
    bd.type          = b2_dynamicBody;
    bd.fixedRotation = false;
    bd.position      = { pos.x, pos.y };
    bd.linearDamping = 2.f;
    bd.isBullet      = true; // CCD — don't tunnel through the thin cup walls
    bd.userData      = reinterpret_cast<void*>(static_cast<uintptr_t>(ent.entity().id));
    b2BodyId body = b2CreateBody(physics.world(), &bd);

    // Solid circle. Reuses the LIQUID category so the cup interior sensor
    // (MASK_CUP_INSIDE == LIQUID), the cup walls (MASK_CUP_SOLID includes LIQUID),
    // and the cleanup sensor (MASK_CLEANUP == LIQUID) all already interact with it.
    // enableSensorEvents: the visitor side of a sensor pair must opt in (Box2D 3.x).
    b2ShapeDef sd = b2DefaultShapeDef();
    sd.density              = 12.f;
    sd.material.friction    = 0.5f;
    sd.material.restitution = 0.05f;
    sd.enableSensorEvents   = true;
    sd.filter.categoryBits  = filter::LIQUID;
    sd.filter.maskBits      = filter::MASK_LIQUID;

    b2Circle circle{ {0.f, 0.f}, r };
    b2CreateCircleShape(body, &sd, &circle);

    // Draggable via DragIntent + Transform only. Deliberately NO DragItemType and
    // NO draggable visitor shape: the cube is never "delivered" to a drop space,
    // it is physically dropped into a cup, so it must stay out of the
    // drop-space/delivery pipeline (which keys on DragItemType).
    ent.addAll(
        Ice{},
        Transform{ .x = pos.x, .y = pos.y, .w = r, .h = r },
        Drawable{ tex.get(), tex.getFullSrcRect(), layer::LIQUID },
        PhysicsBody{ body },
        DragIntent{}
    );
    return ent;
}
} // namespace cafe
