#include "LiquidDropFactory.h"
#include "Components.h"
#include "PhysicsContext.h"
#include "PhysicsFilters.h"
#include <box2d/box2d.h>

namespace cafe
{
bagel::Entity createLiquidDrop(WorldPos pos, SDL_Texture* tex)
{
    auto ent = bagel::Entity::create();
    constexpr float r = 0.06f; // ~0.5 px radius. Raise to 0.10f if drops jitter.

    b2BodyDef bd = b2DefaultBodyDef();
    bd.type     = b2_dynamicBody;
    bd.position = { pos.x, pos.y };
    bd.isBullet = true; // CCD — prevents tunneling through the thin cup walls
    bd.userData = reinterpret_cast<void*>(static_cast<uintptr_t>(ent.entity().id));
    b2BodyId body = b2CreateBody(PhysicsContext::world(), &bd);

    // Visitor side of a sensor pair must also opt in to sensor events.
    b2ShapeDef sd = b2DefaultShapeDef();
    sd.density             = 1.f;
    sd.material.friction   = 0.1f;
    sd.enableSensorEvents  = true;
    sd.filter.categoryBits = filter::LIQUID;
    sd.filter.maskBits     = filter::MASK_LIQUID;

    b2Circle circle{ {0.f, 0.f}, r };
    b2CreateCircleShape(body, &sd, &circle);

    float     w{}, h{};
    SDL_FRect src{ 0.f, 0.f, 2.f, 2.f };
    if (tex && SDL_GetTextureSize(tex, &w, &h))
        src = { 0.f, 0.f, w, h };

    ent.addAll(
        Liquid{},
        Transform{ .x = pos.x, .y = pos.y, .w = r, .h = r },
        Drawable{ tex, src },
        PhysicsBody{ body }
    );
    return ent;
}
} // namespace cafe
