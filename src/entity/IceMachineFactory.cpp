#include "IceMachineFactory.h"
#include "AssetManager.h"
#include "Components.h"
#include "RenderLayers.h"
#include "Texture.h"

namespace cafe
{
bagel::Entity createIceMachine(AssetManager& assets, WorldPos pos)
{
    // The Drawable only carries the render layer + Transform into drawSystem,
    // which renders an IceMachine as a solid gray rectangle (placeholder art).
    // particle.png is reused just to give Drawable a valid texture handle.
    static constexpr auto TEX = "particle.png";
    const Texture& tex = assets.getTexture(TEX);

    constexpr float halfW = 1.2f; // world half-extents — tune freely
    constexpr float halfH = 1.4f;

    auto ent = bagel::Entity::create();
    ent.addAll(
        Transform{ .x = pos.x, .y = pos.y, .w = halfW, .h = halfH },
        Drawable{ tex.get(), tex.getFullSrcRect(), layer::STATIC_ON_BARTOP },
        IceMachine{}
    );
    return ent;
}
} // namespace cafe
