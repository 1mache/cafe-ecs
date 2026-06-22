#include "PastryFactory.h"

#include "Components.h"
#include "DragAndDropSystem.h"
#include "PhysicsContext.h"
#include "PhysicsFilters.h"
#include "RenderLayers.h"
#include "SpriteDims.h"
#include "SpriteSheet.h"
#include "Utils.h"
#include "box2d/types.h"

namespace cafe
{
namespace
{
constexpr auto TEX_PROPS_PATH = "props.png";
constexpr auto SPRITE_DATA    = "props.json";

SDL_FRect getRandomPastryRect(const SpriteSheet& sheet)
{
    auto [from, to] = sheet.getTagBounds("pastry");
    std::uniform_int_distribution dist(from, to);
    return sheet.getFrame(dist(getRng()));
}
} // namespace

// Creates a kinematic pastry entity sitting on the counter, draggable.
bagel::Entity createPastry(AssetManager& assets, PhysicsContext& physics, WorldPos pos)
{

    using namespace cafe;

    const float halfW = screenToWorldScale(PROP_DIMS.x);
    const float halfH = screenToWorldScale(PROP_DIMS.y);

    auto ent = bagel::Entity::create();

    b2BodyDef bd = b2DefaultBodyDef();
    bd.type      = b2_dynamicBody;
    bd.position  = { pos.x, pos.y };
    bd.userData  = reinterpret_cast<void*>(static_cast<uintptr_t>(ent.entity().id));
    b2BodyId body = b2CreateBody(physics.world(), &bd);

    b2ShapeDef solidDef = b2DefaultShapeDef();
    solidDef.material.friction = 0.1f;
    solidDef.material.restitution = 0.5f;
    solidDef.filter.categoryBits = filter::DRAGGABLE;
    solidDef.filter.maskBits     = filter::MASK_DRAGGABLE;
    b2Polygon solidBox = b2MakeBox(halfW, halfH);
    b2CreatePolygonShape(body, &solidDef, &solidBox);

    addDraggableVisitorShape(body, halfW, halfH);

    const SpriteSheet& props = assets.getSpriteSheet(TEX_PROPS_PATH, SPRITE_DATA);
    SDL_FRect src = getRandomPastryRect(props);
    auto&     propsTex = assets.getTexture(TEX_PROPS_PATH);
    ent.addAll(
        Transform{ .x = pos.x, .y = pos.y, .w = halfW, .h = halfH },
        Drawable{ propsTex.get(), src, layer::PROP },
        PhysicsBody{ body },
        DragIntent{},
        DragItemType{ .dropType = DropType::Pastry }
    );
    return ent;
}
}