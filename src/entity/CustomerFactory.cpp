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

    // Order icons sit in a single left-aligned row of up to MAX_ICONS slots.
    // Drinks default to Hot and pastries to Cold; a temperature icon is shown
    // only for the non-default case — ice next to a cold drink, fire next to a
    // hot pastry. Surplus icons past MAX_ICONS are dropped (right side left blank).
    constexpr int MAX_ICONS = 5;
    int frames[MAX_ICONS];
    int n = 0;
    auto push = [&](int f) { if (n < MAX_ICONS) frames[n++] = f; };

    for (int i = 0; i < order.drinkCount; ++i)
    {
        const DrinkItem& d = order.drinks[i];
        push(recipeFor(d.type).iconFrame);
        if (d.temp == Temperature::Cold)
            push(temperatureFrame(Temperature::Cold)); // ice
    }
    for (int i = 0; i < order.pastryCount; ++i)
    {
        const PastryItem& p = order.pastries[i];
        push(static_cast<int>(p.type));
        if (p.temp == Temperature::Hot)
            push(temperatureFrame(Temperature::Hot)); // fire / oven
    }

    constexpr float BUBBLE_W  = BUBBLE_DIMS.x;     // full 64 px, unscaled
    constexpr float SLOT_W    = BUBBLE_W / MAX_ICONS;
    constexpr float ICON_SIZE = SLOT_W * 0.85f;    // fits one slot
    for (int i = 0; i < n; ++i)
    {
        // Slot center relative to bubble center (+x = right).
        const float x = -BUBBLE_W * 0.5f + (static_cast<float>(i) + 0.5f) * SLOT_W;
        createOrderIcon(assets, frames[i], ICON_SIZE, ICON_SIZE, bubble, {x, 0.f});
    }

    return customer;
}

} // namespace cafe
