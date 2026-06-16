#include "ButtonFactory.h"

#include "Components.h"
#include "RenderLayers.h"
#include "SpriteDims.h"
#include "Texture.h"

namespace cafe
{
// A button is a plain UI sprite: Transform + Drawable + SpawnButton, with no
// PhysicsBody and no DragIntent, so it can be clicked but never dragged.
bagel::Entity createSpawnButton(AssetManager& assets, WorldPos pos, DropType item)
{
    // Reuse existing art as the icon: the cup sprite for cups, the props strip
    // (frame 0 = pastry) for pastries.
    const char*     texPath = (item == DropType::cup) ? "cup.png" : "props.png";
    const SDL_FPoint dims    = (item == DropType::cup) ? CUP_DIMS : PROP_DIMS;

    const Texture& tex = assets.getTexture(texPath);
    const SDL_FRect src = { 0.f, 0.f, dims.x, dims.y };

    const float halfW = screenToWorldScale(dims.x);
    const float halfH = screenToWorldScale(dims.y);

    auto ent = bagel::Entity::create();
    ent.addAll(
        Transform{ .x = pos.x, .y = pos.y, .w = halfW, .h = halfH },
        Drawable{ tex.get(), src, layer::UI1 },
        SpawnButton{ .item = item }
    );
    return ent;
}
} // namespace cafe
