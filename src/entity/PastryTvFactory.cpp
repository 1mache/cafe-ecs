#include "PastryTvFactory.h"

#include "AssetManager.h"
#include "Components.h"
#include "RenderLayers.h"
#include "SpriteSheet.h"
#include "Texture.h"
#include "Transform.h" // texToWorldScale
#include <bagel.h>

namespace cafe
{
namespace
{
constexpr auto PROPS_TEX  = "props.png";
constexpr auto PROPS_DATA = "props.json";

// Pastry icon size on the screen, as a fraction of the TV frame (tunable).
constexpr float SCREEN_FRACTION = 0.55f;
// Overall TV size relative to its native sprite size (tunable).
constexpr float TV_SCALE = 0.75f;
} // namespace

void createPastryTv(AssetManager& assets, WorldPos pos)
{
    // --- TV frame: static, purely decorative ---
    const Texture& tvTex = assets.getTexture(PASTRY_TV_TEX);
    const auto [tvW, tvH] = tvTex.getSize();
    const float tvHalfW = texToWorldScale(tvW) * TV_SCALE;
    const float tvHalfH = texToWorldScale(tvH) * TV_SCALE;

    bagel::Entity::create().addAll(
        Transform{ .x = pos.x, .y = pos.y, .w = tvHalfW, .h = tvHalfH },
        Drawable{ tvTex.get(), tvTex.getFullSrcRect(), layer::UI1 });

    // --- Pastry display: the entity the system ticks and mutates ---
    const SpriteSheet& props     = assets.getSpriteSheet(PROPS_TEX, PROPS_DATA);
    const int          pastryFrom = props.getTagBounds("pastry").first;
    const Texture&     propsTex   = assets.getTexture(PROPS_TEX);

    bagel::Entity::create().addAll(
        Transform{ .x = pos.x, .y = pos.y,
                   .w = tvHalfW * SCREEN_FRACTION, .h = tvHalfH * SCREEN_FRACTION },
        Drawable{ propsTex.get(), props.getFrameRect(pastryFrom), layer::UI2 },
        PastryTv{});
}
} // namespace cafe