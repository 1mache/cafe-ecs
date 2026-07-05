#include "PastryTvFactory.h"

#include "AssetManager.h"
#include "Components.h"
#include "RenderLayers.h"
#include "SpawnButtonFactory.h" // createSpawnButton
#include "SpriteSheet.h"
#include "Texture.h"
#include "Transform.h" // texToWorldScale
#include "Utils.h"     // getRng
#include <algorithm>   // std::shuffle
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

void createPastryTv(AssetManager& assets, PhysicsContext& physics, WorldPos pos)
{
    // --- TV frame: static, purely decorative ---
    const Texture& tvTex = assets.getTexture(PASTRY_TV_TEX);
    const auto [tvW, tvH] = tvTex.getSize();
    const float tvHalfW = texToWorldScale(tvW) * TV_SCALE;
    const float tvHalfH = texToWorldScale(tvH) * TV_SCALE;

    bagel::Entity::create().addAll(
        Transform{ .x = pos.x, .y = pos.y, .w = tvHalfW, .h = tvHalfH },
        Drawable{ tvTex.get(), tvTex.getFullSrcRect(), layer::UI1 });

    // --- Pastry display: the entity pastryTvSystem ticks and mutates. It is also
    // the pastry spawn button, so createSpawnButton sets up SpawnButton + the spawn
    // slot sensor; we decorate it here with the rotating pastry icon + PastryTv. ---
    const SpriteSheet& props     = assets.getSpriteSheet(PROPS_TEX, PROPS_DATA);
    const int          pastryFrom = props.getTagBounds("pastry").first;
    const Texture&     propsTex   = assets.getTexture(PROPS_TEX);

    // A fresh random line each TV (re)creation — i.e. every new day/game.
    PastryTv tv{};
    for (int i = 0; i < static_cast<int>(PastryType::count); ++i)
        tv.queue[i] = static_cast<PastryType>(i);
    std::shuffle(std::begin(tv.queue), std::end(tv.queue), getRng());

    auto icon = createSpawnButton(assets, physics, pos, DropType::Pastry);
    icon.addAll(
        Transform{ .x = pos.x, .y = pos.y,
                   .w = tvHalfW * SCREEN_FRACTION, .h = tvHalfH * SCREEN_FRACTION },
        Drawable{ propsTex.get(),
                  props.getFrameRect(pastryFrom + static_cast<int>(tv.queue[0])), layer::UI2 },
        tv);
}
} // namespace cafe