#include "IceMachineFactory.h"
#include "AssetManager.h"
#include "Components.h"
#include "RenderLayers.h"
#include "Texture.h"

namespace cafe
{
bagel::Entity createIceMachine(AssetManager& assets, WorldPos pos)
{
    static constexpr auto TEX = "ice_machine.png";
    const Texture& tex = assets.getTexture(TEX);

    float halfW = texToWorldScale(tex.getSize().x); // world half-extents — tune freely
    float halfH = texToWorldScale(tex.getSize().y);

    auto ent = bagel::Entity::create();
    ent.addAll(
        Transform{ .x = pos.x, .y = pos.y, .w = halfW, .h = halfH },
        Drawable{ tex.get(), tex.getFullSrcRect(), layer::STATIC_ON_BARTOP },
        IceMachine{}
    );
    return ent;
}
} // namespace cafe
