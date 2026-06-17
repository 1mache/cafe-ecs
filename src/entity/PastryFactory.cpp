#include "PastryFactory.h"

#include "Components.h"
#include "DragAndDropSystem.h"
#include "PhysicsContext.h"
#include "PhysicsFilters.h"
#include "RenderLayers.h"
#include "SpriteDims.h"
#include "box2d/types.h"

namespace cafe
{
// Creates a kinematic pastry entity sitting on the counter, draggable.
bagel::Entity createPastry(PhysicsContext& physics, WorldPos pos, AssetManager& assets)
{
    constexpr auto TEX_PROPS_PATH = "props.png";

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

    // Frame 0 of the 3-frame props strip = cinnamon roll.
    SDL_FRect src = { 0.f, 0.f, PROP_DIMS.x, PROP_DIMS.y };
    auto&     propsTex = assets.getTexture(TEX_PROPS_PATH);
    ent.addAll(
        Transform{ .x = pos.x, .y = pos.y, .w = halfW, .h = halfH },
        Drawable{ propsTex.get(), src, layer::PROP },
        PhysicsBody{ body },
        DragIntent{},
        DragItemType{ .dropType = DropType::Pastry },
        HomeSlot{ pos }
    );
    return ent;
}
}