#include "CafeEnvironmentFactory.h"
#include "AssetManager.h"
#include "Components.h"
#include "PhysicsContext.h"
#include "PhysicsFilters.h"
#include "RenderLayers.h"
#include "Texture.h"

namespace cafe
{

namespace
{
static constexpr auto TEX_BARTOP = "counter.png";
}

bagel::Entity createBg(AssetManager& assets, std::string_view bgPath)
{
    auto& bgTex = assets.getTexture(bgPath);
    const auto bgSrcRect = bgTex.getFullSrcRect();

    auto bgEnt = bagel::Entity::create();
    bgEnt.addAll(Drawable{bgTex.get(), bgSrcRect, layer::BG},
                 Transform{.x = 0.f,
                           .y = 0.f,
                           .w = texToWorldScale(LOGICAL_W),
                           .h = texToWorldScale(LOGICAL_H)});
    return bgEnt;
}
bagel::Entity createBartop(AssetManager& assets, PhysicsContext& physicsContext)
{
    const Texture& bartopTex     = assets.getTexture(TEX_BARTOP);
    const auto  bartopSrcRect    = bartopTex.getFullSrcRect();
    const float bartopHalfWidth  = texToWorldScale(bartopSrcRect.w);
    const float bartopHalfHeight = texToWorldScale(bartopSrcRect.h);
    const float bartopY = -(texToWorldDistance(LOGICAL_H / 2) - bartopHalfHeight);
    auto  ent        = bagel::Entity::create();

    auto world = physicsContext.world();
    b2BodyDef bd = b2DefaultBodyDef();
    bd.type = b2_staticBody;
    bd.position = { 0.f, bartopY };
    bd.userData  = reinterpret_cast<void*>(static_cast<uintptr_t>(ent.entity().id));
    auto bartopBodyId = b2CreateBody(world, &bd);

    // Polygon covering the top 10% of the texture (a strip at the top edge).
    const float stripHalfHeight = 0.1f * bartopHalfHeight; // 10% of full height
    const float stripOffsetY     = bartopHalfHeight - stripHalfHeight - texToWorldDistance(16.f); // top, body-local
    const b2Polygon topStrip =
        b2MakeOffsetBox(bartopHalfWidth, stripHalfHeight, { 0.f, stripOffsetY }, b2Rot_identity);

    b2ShapeDef bartopShape = b2DefaultShapeDef();
    bartopShape.material.friction = 0.3f;
    bartopShape.filter.categoryBits = filter::BARTOP;
    bartopShape.filter.maskBits     = filter::MASK_BARTOP;
    b2CreatePolygonShape(bartopBodyId, &bartopShape, &topStrip);

    ent.addAll(
        Drawable{bartopTex.get(), bartopSrcRect, layer::BARTOP},
        Transform{.x = 0.f,
                  .y = bartopY,
                  .w = bartopHalfWidth,
                  .h = bartopHalfHeight},
        PhysicsBody{bartopBodyId});

    return ent;
}
} // namespace cafe