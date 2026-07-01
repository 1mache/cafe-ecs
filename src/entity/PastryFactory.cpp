#include "PastryFactory.h"

#include "Components.h"
#include "DragAndDropSystem.h"
#include "Pastry.h"
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

PastryType getRandomPastryType()
{
    std::uniform_int_distribution dist(0, static_cast<int>(PastryType::count) - 1);
    return static_cast<PastryType>(dist(getRng()));
}

SDL_FRect getSpriteFromType(const SpriteSheet& sheet, PastryType type)
{
    return sheet.getFrameRect(static_cast<int>(type));
}
} // namespace

// Creates a kinematic pastry entity sitting on the counter, draggable.
bagel::Entity createPastry(AssetManager& assets, PhysicsContext& physics, WorldPos pos,
                           PastryType type)
{

    using namespace cafe;

    const float halfW = texToWorldScale(PROP_DIMS.x);
    const float halfH = texToWorldScale(PROP_DIMS.y);

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

    const SpriteSheet& propsSheet = assets.getSpriteSheet(TEX_PROPS_PATH, SPRITE_DATA);
    if (type == PastryType::count)
        type = getRandomPastryType();
    SDL_FRect srcRect = getSpriteFromType(propsSheet,type);
    auto&     propsTex = assets.getTexture(TEX_PROPS_PATH);
    ent.addAll(
        Transform{ .x = pos.x, .y = pos.y, .w = halfW, .h = halfH },
        Drawable{ propsTex.get(), srcRect, layer::PROP },
        PhysicsBody{ body },
        DragIntent{},
        DragItemType{ .dropType = DropType::Pastry },
        Pastry(type)
    );
    return ent;
}
}