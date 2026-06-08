#include "CoffeeMachineFactory.h"
#include "Components.h"
#include "GameConfig.h"
#include "PhysicsContext.h"
#include <box2d/box2d.h>

namespace cafe
{
bagel::Entity createCoffeeMachine(WorldPos pos, WorldPos spoutOffset,
                                  SDL_Texture* tex, float texW, float texH)
{
    auto ent = bagel::Entity::create();

    const float halfW = texW / (2.f * PTM);
    const float halfH = texH / (2.f * PTM);
    Transform t{ .x = pos.x, .y = pos.y, .w = halfW, .h = halfH };

    // Kinematic body, no fixtures — the machine is just a position anchor for spawning.
    b2BodyDef bd = b2DefaultBodyDef();
    bd.type     = b2_kinematicBody;
    bd.position = { t.x, t.y };
    bd.userData = reinterpret_cast<void*>(static_cast<uintptr_t>(ent.entity().id));
    b2BodyId body = b2CreateBody(PhysicsContext::world(), &bd);

    ent.addAll(
        t,
        Drawable{ tex, { 0.f, 0.f, texW, texH } },
        PhysicsBody{ body },
        CoffeeSpawner{
            .interval    = 0.05f,
            .accumulator = 0.f,
            .active      = false,
            .offset      = spoutOffset
        }
    );
    return ent;
}
} // namespace cafe
