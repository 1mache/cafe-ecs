#include "NapkinFactory.h"

#include "AssetManager.h"
#include "Components.h"
#include "PhysicsContext.h"
#include "RenderLayers.h"
#include "Texture.h"

#include <box2d/box2d.h>

namespace cafe
{
bagel::Entity createNapkin(AssetManager& assets, PhysicsContext& physics)
{
    static constexpr auto TEX = "napkin.png";
    const Texture& tex = assets.getTexture(TEX);
    const SDL_FRect src = tex.getFullSrcRect();

    auto ent = bagel::Entity::create();

    b2BodyDef bd = b2DefaultBodyDef();
    bd.type          = b2_dynamicBody;
    bd.position      = { NAPKIN_HIDDEN_CENTER_X, NAPKIN_HIDDEN_CENTER_Y };
    bd.userData      = reinterpret_cast<void*>(static_cast<uintptr_t>(ent.entity().id));
    bd.fixedRotation = true;
    b2BodyId body = b2CreateBody(physics.world(), &bd);
    b2Body_SetGravityScale(body, 0.f);

    ent.addAll(
        Transform{ .x = NAPKIN_HIDDEN_CENTER_X,
                   .y = NAPKIN_HIDDEN_CENTER_Y,
                   .w = NAPKIN_HIDDEN_HALF_W,
                   .h = NAPKIN_HIDDEN_HALF_H },
        Drawable{ tex.get(), src, layer::UI1 },
        NapkinIntent{},
        PhysicsBody{ body });
    return ent;
}
} // namespace cafe
