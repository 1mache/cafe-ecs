#include "AssetManager.h"
#include "RenderLayers.h"
#include "Components.h"
#include "CustomerFactory.h"

#include "Animation.h"
#include "Menu.h"
#include "OrderIconFactory.h"
#include "PhysicsContext.h"
#include "PhysicsFilters.h"
#include "SpeechBubbleFactory.h"
#include "SpriteDims.h"
#include "SpriteSheet.h"
#include "Texture.h"
#include "Utils.h"
#include <bagel.h>
#include <cassert>


namespace cafe
{
namespace
{
constexpr auto TEX          = "customers.png";
constexpr auto SPRITE_DATA  = "customers.json";


// from the spritesheet takes a random customer sprite
int getRandomCustomerId(const SpriteSheet& spriteSheet)
{
    constexpr int FRAMES_PER_CUSTOMER = 2;
    int nTags = static_cast<int>(std::ranges::size(spriteSheet.tags()));
    auto dist = std::uniform_int_distribution(0,  nTags - 1);

    return dist(getRng()) * FRAMES_PER_CUSTOMER;
}

constexpr Animation createTalkingAnimation(int customerId)
{
    AnimationFrame frameMouthShut
    {
        .spritesheetIndex = customerId
    };

    AnimationFrame frameMouthOpen
    {
        .spritesheetIndex = customerId + 1
    };

    return Animation
    {
        {frameMouthShut, frameMouthOpen},
        SPRITE_DATA,
        0,
        2,
        frameMouthOpen.duration,
        TALKING_TIME,
        true,
        false
    };
}
}

// Attaches a static DropSpace sensor + OrderGrade to an existing client entity so
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
        OrderGrade{}
    );
}

bagel::Entity createCustomer(AssetManager& assets, PhysicsContext& physics,
                           WorldPos pos, Order order, float patience)
{
    assert((order.hasDrink || order.hasPastry) && "createClient: order must have at least one item");

    const Texture& tex       = assets.getTexture(TEX);
    const SpriteSheet& spriteSheet = assets.getSpriteSheet(TEX, SPRITE_DATA);
    const int customerId = getRandomCustomerId(spriteSheet);
    auto [w, h] = PERSON_DIMS;
    auto ent = bagel::Entity::create();
    ent.addAll(
        Transform{.x = pos.x, .y = pos.y, .w = texToWorldScale(w), .h = texToWorldScale(h)},
        Drawable{tex.get(), spriteSheet.getFrameRect(customerId), layer::CUSTOMER},
        createTalkingAnimation(customerId),
        order,
        Behavior{.patience = patience, .maxPatience = patience});

    makeCustomerDeliverable(physics, ent);

    return ent;
}

bagel::Entity spawnCustomer(AssetManager& assets, PhysicsContext& physics,
                            WorldPos pos, Order order, float patience)
{
    auto customer = createCustomer(assets, physics, pos, order, patience);

    // Speech bubble is a child of the customer; order icons are children of the bubble.
    auto bubble = createSpeechBubble(assets, customer, { 0.f, PERSON_DIMS.y/2.f  + 0.5f * (BUBBLE_DIMS.y/2.f)});

    // Order icons sit in a single left-aligned row of up to MAX_ORDER_ICONS slots.
    // Drinks default to Hot and pastries to Cold; a temperature icon is shown
    // only for the non-default case — ice next to a cold drink, fire next to a
    // hot pastry. Orders are trimmed to fit, so the push cap is just a backstop.
    constexpr auto PROPS_TEX         = "props.png";
    constexpr auto PROPS_SPRITE_DATA = "props.json";
    const Texture&     propsTex = assets.getTexture(PROPS_TEX);
    const SpriteSheet& props    = assets.getSpriteSheet(PROPS_TEX, PROPS_SPRITE_DATA);
    const int coffeeFrom = props.getTagBounds("coffee").first;
    const int pastryFrom = props.getTagBounds("pastry").first;
    const int checkFrame = props.getTagBounds("status").first; // green checkmark

    // Patience dial: badge on the bubble's top-left corner, drained by patienceDialSystem.
    constexpr float      DIAL_SIZE   = 10.f; // px, ~order-icon size
    constexpr SDL_FPoint DIAL_OFFSET = { -BUBBLE_DIMS.x / 2.f, BUBBLE_DIMS.y / 2.f - 4.f };
    const int patienceFrom = props.getTagBounds("patience").first;
    bagel::Entity::create().addAll(
        Transform{ .w = texToWorldScale(DIAL_SIZE), .h = texToWorldScale(DIAL_SIZE) },
        Drawable{ propsTex.get(), props.getFrameRect(patienceFrom), layer::UI3 },
        ChildOf{ bubble, DIAL_OFFSET },
        PatienceDial{});

    // Each icon carries its frame and, for a main item icon, the order slot it
    // represents (drinkSlot/pastrySlot < 0 for temperature icons). The slot lets
    // us drop a hidden checkmark on top, revealed once that slot is served.
    struct IconSpec { int frame; int drinkSlot; int pastrySlot; };
    IconSpec icons[MAX_ORDER_ICONS];
    int n = 0;
    auto push = [&](int f, int drinkSlot, int pastrySlot)
    { if (n < MAX_ORDER_ICONS) icons[n++] = { f, drinkSlot, pastrySlot }; };

    for (int i = 0; i < order.drinkCount; ++i)
    {
        const Order::DrinkItem& d = order.drinks[i];
        push(coffeeFrom + static_cast<int>(d.type), i, -1);
        if (d.temp == Temperature::Cold)
            push(temperatureFrame(Temperature::Cold), -1, -1); // ice
    }
    for (int i = 0; i < order.pastryCount; ++i)
    {
        const Order::PastryItem& p = order.pastries[i];
        push(pastryFrom + static_cast<int>(p.type), -1, i);
        if (p.temp == Temperature::Hot)
            push(temperatureFrame(Temperature::Hot), -1, -1); // fire / oven
    }

    constexpr float BUBBLE_W  = BUBBLE_DIMS.x;     // full 64 px, unscaled
    constexpr float MARGIN    = 0.05f * BUBBLE_W;
    constexpr float BETWEEN   = 0.02f * BUBBLE_W;
    constexpr float SLOT_W    = (BUBBLE_W - ((MAX_ORDER_ICONS-1) * BETWEEN) - (2 * MARGIN)) / MAX_ORDER_ICONS;
    for (int i = 0; i < n; ++i)
    {
        // Slot center relative to bubble center (+x = right).
        const float x = -(BUBBLE_W * 0.5f) + MARGIN + SLOT_W * 0.5f + static_cast<float>(i) * (SLOT_W + BETWEEN);
        createOrderIcon(assets, icons[i].frame, SLOT_W, SLOT_W, bubble, {x, 0.f});

        // Main item icons get a checkmark overlay, spawned hidden (alpha 0) at the
        // same slot; OrderCheckmarkSystem reveals it once the slot is served.
        const bool isDrink = icons[i].drinkSlot >= 0;
        const int  slot    = isDrink ? icons[i].drinkSlot : icons[i].pastrySlot;
        if (slot < 0) continue;
        bagel::Entity::create().addAll(
            Transform{.w = texToWorldScale(SLOT_W), .h = texToWorldScale(SLOT_W)},
            Drawable{propsTex.get(), props.getFrameRect(checkFrame), layer::UI3,
                     SDL_Color{255, 255, 255, 0}},
            ChildOf{bubble, {x, 0.f}},
            Checkmark{isDrink, slot});
    }

    return customer;
}

} // namespace cafe
