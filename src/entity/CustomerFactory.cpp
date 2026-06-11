#include "AssetManager.h"
#include "RenderLayers.h"
#include "Components.h"
#include "CustomerFactory.h"
#include "PhysicsContext.h"
#include "PhysicsFilters.h"
#include "Texture.h"
#include <bagel.h>
#include <cassert>

namespace cafe
{
// Attaches a static DropSpace sensor + Served to an existing client entity so
// dragged items can be "dropped on" the customer.
void makeCustomerDeliverable(bagel::Entity client)
{
    using namespace cafe;

    const auto& t = client.get<Transform>();

    b2BodyDef bd = b2DefaultBodyDef();
    bd.type      = b2_staticBody;
    bd.position  = { t.x, t.y };
    bd.userData  = reinterpret_cast<void*>(static_cast<uintptr_t>(client.entity().id));
    b2BodyId body = b2CreateBody(PhysicsContext::world(), &bd);

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

bagel::Entity createCustomer(AssetManager& assets,
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

    makeCustomerDeliverable(ent);

    return ent;
}

} // namespace cafe
