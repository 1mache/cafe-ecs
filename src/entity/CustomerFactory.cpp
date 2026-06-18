#include "AssetManager.h"
#include "RenderLayers.h"
#include "Components.h"
#include "CustomerFactory.h"
#include "Menu.h"
#include "OrderIconFactory.h"
#include "PhysicsContext.h"
#include "PhysicsFilters.h"
#include "SpeechBubbleFactory.h"
#include "SpriteDims.h"
#include "Texture.h"
#include <bagel.h>
#include <cassert>

namespace cafe
{
// Attaches a static DropSpace sensor + Served to an existing client entity so
// dragged items can be "dropped on" the customer.
void makeCustomerDeliverable(PhysicsContext& physics, bagel::Entity client)
{
    using namespace cafe;

    const auto& t = client.get<Transform>();

    b2BodyDef bd = b2DefaultBodyDef();
    bd.type      = b2_staticBody;
    bd.position  = { t.x, t.y };
    bd.userData  = reinterpret_cast<void*>(static_cast<uintptr_t>(client.entity().id));
    b2BodyId body = b2CreateBody(physics.world(), &bd);

    b2ShapeDef sensor          = b2DefaultShapeDef();
    sensor.isSensor            = true;
    sensor.enableSensorEvents  = true;
    sensor.filter.categoryBits = filter::DROPSPACE_SENSOR;
    sensor.filter.maskBits     = filter::MASK_DROPSPACE_SENSOR;
    b2Polygon zone = b2MakeOffsetBox(t.w, t.h, { 0.f, 0.f }, b2Rot_identity);
    b2CreatePolygonShape(body, &sensor, &zone);

    client.addAll(
        PhysicsBody{ body },
        DropSpace{ DropType::Any },
        Served{}
    );
}

bagel::Entity createCustomer(AssetManager& assets, PhysicsContext& physics,
                           WorldPos pos, Order order, float patience)
{
    assert((order.hasDrink || order.hasPastry) && "createClient: order must have at least one item");
    constexpr auto TEX = "def_customer.png";

    const Texture& tex = assets.getTexture(TEX);
    auto [w, h] = tex.getSize();
    auto ent = bagel::Entity::create();
    ent.addAll(
        Transform{.x = pos.x, .y = pos.y, .w = screenToWorldScale(w), .h = screenToWorldScale(h)},
        Drawable{tex.get(), tex.getFullSrcRect(), layer::CUSTOMER},
        order,
        Behavior{.patience = patience});

    makeCustomerDeliverable(physics, ent);

    return ent;
}

bagel::Entity spawnCustomer(AssetManager& assets, PhysicsContext& physics,
                            WorldPos pos, Order order, float patience)
{
    auto customer = createCustomer(assets, physics, pos, order, patience);

    // Speech bubble is a child of the customer; order icons are children of the bubble.
    auto bubble = createSpeechBubble(assets, customer, { 0.f, 28.f });

    // The bubble is split into 4 equal columns (quarters of its width):
    //   0: beverage   1: beverage temp   2: pastry   3: pastry temp
    // Items stack vertically within their column (row 0 top -> row 2 bottom),
    // up to BUBBLE_MAX_ICONS_PER_COLUMN per column.
    constexpr int   BUBBLE_MAX_ICONS_PER_COLUMN = 3;
    constexpr float BUBBLE_W  = BUBBLE_DIMS.x * BUBBLE_SCALE; // on-screen px
    constexpr float BUBBLE_H  = BUBBLE_DIMS.y * BUBBLE_SCALE; // on-screen px
    constexpr float COL_W     = BUBBLE_W / 4.f;
    constexpr float ROW_H     = BUBBLE_H / BUBBLE_MAX_ICONS_PER_COLUMN;
    // Icon must fit both a quarter-width column and one stacked row, then 0.7x.
    constexpr float ICON_SIZE = (COL_W < ROW_H ? COL_W : ROW_H) * 0.7f;
    // Pull icon centers toward the bubble center to tighten the gaps between them.
    constexpr float GAP       = 0.8f;
    constexpr float COL_DX    = COL_W * GAP;
    constexpr float ROW_DY    = ROW_H * GAP;
    // Column centers, left -> right, relative to bubble center (+x = right).
    constexpr float COL_X[4]  = {-1.5f * COL_DX, -0.5f * COL_DX,
                                 +0.5f * COL_DX, +1.5f * COL_DX};
    // Row centers, top -> bottom, relative to bubble center (+y = up).
    constexpr float ROW_Y[BUBBLE_MAX_ICONS_PER_COLUMN] = {+ROW_DY, 0.f, -ROW_DY};

    for (int i = 0; i < order.drinkCount && i < BUBBLE_MAX_ICONS_PER_COLUMN; ++i)
    {
        const DrinkItem& d = order.drinks[i];
        // Column 0: beverage icon — this drink's coffee frame in props.png.
        createOrderIcon(assets, recipeFor(d.type).iconFrame, ICON_SIZE, ICON_SIZE,
                        bubble, {COL_X[0], ROW_Y[i]});
        // Column 1: beverage temperature — fire (Hot) or ice (Cold).
        createOrderIcon(assets, temperatureFrame(d.temp), ICON_SIZE, ICON_SIZE,
                        bubble, {COL_X[1], ROW_Y[i]});
    }

    for (int i = 0; i < order.pastryCount && i < BUBBLE_MAX_ICONS_PER_COLUMN; ++i)
    {
        const PastryItem& p = order.pastries[i];
        // Column 2: pastry icon — this pastry's frame in props.png.
        createOrderIcon(assets, static_cast<int>(p.type), ICON_SIZE, ICON_SIZE,
                        bubble, {COL_X[2], ROW_Y[i]});
        // Column 3: pastry temperature — fire (Hot) or ice (Cold).
        createOrderIcon(assets, temperatureFrame(p.temp), ICON_SIZE, ICON_SIZE,
                        bubble, {COL_X[3], ROW_Y[i]});
    }

    return customer;
}

} // namespace cafe
