#include "CoffeeMachineFactory.h"
#include "AssetManager.h"
#include "Components.h"
#include "GameConfig.h"
#include "PhysicsContext.h"
#include "Texture.h"

#include <box2d/box2d.h>

namespace cafe
{
namespace
{
static constexpr auto TEX = "machine.png";
}

bagel::Entity createCoffeeMachine(AssetManager& assets,
                                  WorldPos      pos,
                                  WorldPos      spoutOffset)
{
    const Texture& tex = assets.getTexture(TEX);
    auto [x, y] = tex.getSize();
    auto ent = bagel::Entity::create();

    const float halfW = screenToWorldScale(x);
    const float halfH = screenToWorldScale(y);
    Transform t{ .x = pos.x, .y = pos.y, .w = halfW, .h = halfH };

    // Kinematic body, no fixtures — the machine is just a position anchor for spawning.
    b2BodyDef bd = b2DefaultBodyDef();
    bd.type     = b2_kinematicBody;
    bd.position = { t.x, t.y };
    bd.userData = reinterpret_cast<void*>(static_cast<uintptr_t>(ent.entity().id));
    b2BodyId body = b2CreateBody(PhysicsContext::world(), &bd);

    ent.addAll(
        t,
        Drawable{ tex.get(), tex.getFullSrcRect(), LAYER_STATIC_ON_BARTOP },
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
