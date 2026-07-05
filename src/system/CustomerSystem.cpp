#include "CustomerSystem.h"
#include "AssetManager.h"
#include "Components.h"
#include "CustomerFactory.h"
#include "DayState.h"
#include "Entities.h"
#include "MainGameScene.h"
#include "Menu.h"
#include "RenderLayers.h"
#include "SpriteSheet.h"
#include <algorithm>
#include <bagel.h>
#include <box2d/box2d.h>
#include <cmath>
#include <iostream>
#include <vector>

namespace cafe
{
// Patience scales with prep effort: each item adds seconds by how long it takes
// to make — a cold drink needs the extra cooling step, a hot pastry a microwave trip.
static constexpr float PATIENCE_BASE        = 20.f; // seconds regardless of order size
static constexpr float PATIENCE_HOT_DRINK   = 10.f;
static constexpr float PATIENCE_COLD_DRINK  = 14.f;
static constexpr float PATIENCE_COLD_PASTRY = 10.f;
static constexpr float PATIENCE_HOT_PASTRY  = 16.f; // microwave trip

constexpr float patienceFor(const Order& o)
{
    float p = PATIENCE_BASE;
    for (int i = 0; i < o.drinkCount; ++i)
        p += o.drinks[i].temp == Temperature::Cold ? PATIENCE_COLD_DRINK : PATIENCE_HOT_DRINK;
    for (int i = 0; i < o.pastryCount; ++i)
        p += o.pastries[i].temp == Temperature::Hot ? PATIENCE_HOT_PASTRY : PATIENCE_COLD_PASTRY;
    return p;
}

namespace
{
// --- Grade-popup FX tuning (world units; +y is up on screen). ---
constexpr float POPUP_TEXT_LIFE   = 1.0f;  // seconds
constexpr float POPUP_PART_LIFE   = 0.6f;  // seconds
constexpr float POPUP_RISE        = 3.0f;  // text upward drift (u/s)
constexpr float POPUP_SINK        = -2.0f; // failed text downward drift (u/s)
constexpr float PART_HALF         = 0.15f; // particle quad half-extent (~2.4px)
constexpr float PART_SPEED        = 4.0f;  // particle burst speed (u/s)

struct TierLook
{
    const char* text;
    float       scale;
    SDL_Color   color;
    float       textVelY;   // popup text vertical drift
    int         partCount;  // number of burst quads
    float       partDirY;   // +1 up, -1 down bias for the burst
    SDL_Color   partColor;
};

TierLook lookFor(GradeTier tier)
{
    switch (tier)
    {
        case GradeTier::Perfect:
            return { "PERFECT", 1.f, {255, 210, 60, 255}, POPUP_RISE, 8,  1.f, {255, 230, 120, 255} };
        case GradeTier::Good:
            return { "GOOD",    1.f, {110, 220, 90, 255}, POPUP_RISE, 4,  1.f, {110, 220, 90, 255} };
        case GradeTier::Meh:
            return { "MEH",     1.f, {150, 150, 150, 255}, POPUP_RISE, 3, -1.f, {150, 150, 150, 255} };
        case GradeTier::Failed:
        default:
            return { "TOO SLOW", 1.f, {230, 60, 50, 255}, POPUP_SINK, 5, -1.f, {40, 40, 40, 255} };
    }
}

// Fixed-angle fan velocity for burst particle i of n (deterministic, no RNG).
// Spread across a half-circle biased up or down by dirY.
WorldPos burstVelocity(int i, int n, float dirY)
{
    // Evenly spaced angles from ~20deg to ~160deg, measured from +x, then the
    // vertical component is signed by dirY so "up" bursts rise, "down" sink.
    const float t     = n > 1 ? static_cast<float>(i) / static_cast<float>(n - 1) : 0.5f;
    const float angle = 0.35f + t * 2.44f; // radians, ~20deg..160deg
    const float vx    = PART_SPEED * std::cos(angle);
    const float vy    = PART_SPEED * std::sin(angle) * dirY;
    return { vx, vy };
}

void spawnGradePopup(float cx, float cy, GradeTier tier)
{
    const TierLook look = lookFor(tier);

    // Text popup: TextLabel + Particle (drift/fade) + Lifetime.
    auto textEnt = bagel::Entity::create();
    textEnt.addAll(
        Transform{ .x = cx, .y = cy },
        TextLabel{ look.text, look.scale, TextAlign::Center, look.color },
        Particle{ WorldPos{ 0.f, look.textVelY } },
        Lifetime{ 0.f, POPUP_TEXT_LIFE });

    // Burst quads: null-texture Drawable (drawn as a tint rect) + Particle + Lifetime.
    for (int i = 0; i < look.partCount; ++i)
    {
        auto q = bagel::Entity::create();
        q.addAll(
            Transform{ .x = cx, .y = cy, .w = PART_HALF, .h = PART_HALF },
            Drawable{ nullptr, SDL_FRect{}, layer::UI2, look.partColor },
            Particle{ burstVelocity(i, look.partCount, look.partDirY) },
            Lifetime{ 0.f, POPUP_PART_LIFE });
    }
}
} // namespace


void customerSpawnerSystem(AssetManager& assets, float dtSeconds)
{
    static const bagel::Mask spawnerMask =
        bagel::MaskBuilder().set<CustomerSpawner>().build();
    static const bagel::Mask customerMask =
        bagel::MaskBuilder().set<Order>().set<Behavior>().build();

    // The seat is "occupied" while any customer exists (through its whole
    // walk-out, since Order+Behavior aren't stripped until destroy).
    int customers = 0;
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
        if (e.test(customerMask)) ++customers;

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(spawnerMask)) continue;
        auto& sp = e.get<CustomerSpawner>();

        // Seat busy: hold the timer armed so the full interval is waited once it frees.
        if (customers > 0)
        {
            sp.cooldown = sp.interval;
            continue;
        }

        sp.cooldown -= dtSeconds;
        if (sp.cooldown <= 0.f)
        {
            const Order order = randomOrder();
            createCustomer(assets, sp.seat, order, patienceFor(order));
            sp.cooldown = sp.interval;
        }
    }
}

void behaviorSystem(float dtSeconds)
{
    static const bagel::Mask behaviorMask =
        bagel::MaskBuilder().set<Behavior>().set<Customer>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(behaviorMask)) continue;

        // Patience only runs while seated (Ordering/Mad) — paused during the
        // walk-in/walk-out legs.
        const auto state = e.get<Customer>().state;
        if (state != CustomerState::Ordering && state != CustomerState::Mad)
            continue;

        auto& behavior = e.get<Behavior>();
        behavior.patience -= dtSeconds;

        if (behavior.patience <= 0.f && !e.has<Leaving>())
            e.add(Leaving{});
    }
}

void patienceDialSystem(AssetManager& assets)
{
    static const bagel::Mask dialMask =
        bagel::MaskBuilder().set<PatienceDial>().set<ChildOf>().set<Drawable>().build();

    const SpriteSheet& props = assets.getSpriteSheet("props.png", "props.json");
    const auto [from, to]    = props.getTagBounds("patience");
    const int steps          = to - from; // frame steps beyond the full-patience frame

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(dialMask)) continue;

        // Walk up the hierarchy: dial -> bubble -> customer (checkmark idiom).
        bagel::Entity bubble = e.get<ChildOf>().parent;
        if (!bubble.has<ChildOf>()) continue;
        bagel::Entity customer = bubble.get<ChildOf>().parent;
        if (!customer.has<Behavior>()) continue;

        const auto& b = customer.get<Behavior>();
        const float frac = b.maxPatience > 0.f
                         ? std::clamp(b.patience / b.maxPatience, 0.f, 1.f) : 0.f;
        const int frame = from + static_cast<int>(std::round((1.f - frac) * static_cast<float>(steps)));
        e.get<Drawable>().srcRect = props.getFrameRect(frame);
    }
}

void deliverySystem()
{
    static const bagel::Mask dragMask =
        bagel::MaskBuilder().set<DragIntent>().build();
    static const bagel::Mask customerMask =
        bagel::MaskBuilder().set<Order>().set<OrderGrade>().set<Transform>().build();

    std::vector<bagel::Entity> customers;
    for (auto c = bagel::Entity::first(); !c.eof(); c.next())
        if (c.test(customerMask))
            customers.push_back(c);

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(dragMask)) continue;

        auto& intent = e.get<DragIntent>();
        if (intent.intentType != DragIntentType::released) continue;

        // Resolve which customer (if any) the item was dropped on.
        // Primary: the sensor-driven dropSpaceEntity. Fallback: geometric overlap.
        // The sensor's begin-touch only fires on a held ENTER transition, so an item
        // that started already inside the zone never gets dropSpaceEntity set; the
        // overlap test catches that case without depending on any event.
        bool            onCustomer = false;
        bagel::ent_type targetId{};
        if (intent.dropSpaceEntity.has_value())
        {
            bagel::Entity t{ *intent.dropSpaceEntity };
            if (t.has<Order>() && t.has<OrderGrade>())
            {
                targetId    = *intent.dropSpaceEntity;
                onCustomer  = true;
            }
        }
        else if (e.has<Transform>())
        {
            const auto& itemT = e.get<Transform>();
            for (auto customer : customers)
                if (boxesOverlap(itemT, customer.get<Transform>()))
                {
                    targetId   = customer.entity();
                    onCustomer = true;
                    break;
                }
        }
        if (!onCustomer) continue;

        bagel::Entity target{ targetId };
        if (!intent.dropSpaceEntity.has_value())
            intent.dropSpaceEntity = target.entity();

        const auto& order = target.get<Order>();
        const auto& grade = target.get<OrderGrade>();

        if (e.has<Cup>())
        {
            if (firstUnservedDrink(order, grade) < 0 || e.has<CheckCoffeeIntent>())
            {
                rejectItem(e);
                makeCustomerMad(target);
                intent.dropSpaceEntity = std::nullopt;
                continue;
            }

            e.add(CheckCoffeeIntent{ .customer = target.entity() });
        }
        else // pastry
        {
            if (firstUnservedPastry(order, grade) < 0 || e.has<CheckPastryIntent>())
            {
                rejectItem(e);
                makeCustomerMad(target);
                intent.dropSpaceEntity = std::nullopt;
                continue;
            }

            e.add(CheckPastryIntent{ .customer = target.entity() });
        }
    }
}

void orderSystem()
{
    static const bagel::Mask orderMask =
        bagel::MaskBuilder().set<Order>().set<Behavior>().set<OrderGrade>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(orderMask)) continue;
        if (e.has<Leaving>()) continue;

        if (allItemsServed(e.get<Order>(), e.get<OrderGrade>()))
            e.add(Leaving{});
    }
}

void finalizeOrderGradeSystem()
{
    static const bagel::Mask mask =
        bagel::MaskBuilder().set<Leaving>().set<Behavior>().set<Order>().set<OrderGrade>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask)) continue;

        auto& behavior       = e.get<Behavior>();
        const auto& order    = e.get<Order>();
        const auto& grade    = e.get<OrderGrade>();

        behavior.succeeded = allItemsServed(order, grade);

        const int raw = sumItemGrades(order, grade);

        // Patience penalty: two steps, capped at 25% reduction.
        float factor = 1.0f;
        if (behavior.patience <= 0.f)
            factor = 0.75f;
        else if (behavior.maxPatience > 0.f && behavior.patience <= 0.5f * behavior.maxPatience)
            factor = 0.875f;

        behavior.rating = static_cast<int>(std::round(static_cast<float>(raw) * factor));
    }
}

void gradePopupSystem()
{
    static const bagel::Mask mask =
        bagel::MaskBuilder().set<Leaving>().set<Behavior>().set<Order>()
                            .set<OrderGrade>().set<Transform>().build();

    // Snapshot first (spawning new entities mid-iteration is unsafe).
    struct Spawn { float x, y; GradeTier tier; };
    std::vector<Spawn> spawns;

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask)) continue;

        const Behavior&  b     = e.get<Behavior>();
        const Order&     order = e.get<Order>();
        const Transform& t     = e.get<Transform>();

        const int       items = order.drinkCount + order.pastryCount;
        const GradeTier tier  = gradeTier(b.rating, items, b.succeeded);
        spawns.push_back({ t.x, t.y, tier });
    }

    for (const auto& s : spawns)
        spawnGradePopup(s.x, s.y, s.tier);
}

void makeCustomerMad(bagel::Entity customer)
{
    if (!customer.has<Customer>()) return;
    auto& c = customer.get<Customer>();
    if (c.state != CustomerState::Ordering) return;

    c.state    = CustomerState::Mad;
    c.madTimer = CUSTOMER_MAD_TIME;
}

void calmCustomer(bagel::Entity customer)
{
    if (!customer.has<Customer>()) return;
    auto& c = customer.get<Customer>();
    if (c.state != CustomerState::Mad) return;

    c.state = CustomerState::Ordering; // sprite reset next customerStateSystem tick
}

void customerStateSystem(AssetManager& assets, PhysicsContext& physics, float dt)
{
    static const bagel::Mask mask = bagel::MaskBuilder().set<Customer>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask)) continue;

        auto& customer = e.get<Customer>();

        // Departure edge: fires exactly once, the frame Leaving first appears
        // on a still-arriving/seated/mad customer.
        if (e.has<Leaving>() && customer.state != CustomerState::Departing)
        {
            const auto& behavior = e.get<Behavior>();

            if (e.has<PhysicsBody>())
            {
                b2DestroyBody(e.get<PhysicsBody>().id);
                e.del<PhysicsBody>();
            }
            if (e.has<DropSpace>())  e.del<DropSpace>();
            if (e.has<OrderGrade>()) e.del<OrderGrade>();

            if (customer.bubble.entity().id >= 0)
                customer.bubble.addAll(Destroy{});

            const Transform& t = e.get<Transform>();
            e.add(Tween{ .original = t,
                         .target   = Transform{ .x = CUSTOMER_EXIT.x, .y = CUSTOMER_EXIT.y, .w = t.w, .h = t.h },
                         .kind     = Tween::Smooth, .duration = CUSTOMER_WALK_OUT_DURATION });

            DayState::record(behavior.succeeded, behavior.rating);
            if (behavior.succeeded)
                std::cout << "[Order] Customer left SUCCESSFUL — rating: " << behavior.rating << "\n";
            else
                std::cout << "[Order] Customer left FAILED (patience ran out) — rating: " << behavior.rating << "\n";

            customer.state = CustomerState::Departing;
            continue;
        }

        switch (customer.state)
        {
        case CustomerState::Arriving:
            if (!e.has<Tween>()) // walk-in finished
            {
                customer.bubble = attachOrderBubble(assets, e);
                e.add(createTalkingAnimation(customer.spriteBase));
                makeCustomerDeliverable(physics, e);
                customer.state = CustomerState::Ordering;
            }
            break;

        case CustomerState::Mad:
            if (!customer.showingMad)
            {
                if (e.has<Animation>()) e.del<Animation>(); // stop talking (may have already self-ended)
                setCustomerFrame(assets, e, customer.spriteBase + 2);
                customer.showingMad = true;
            }
            customer.madTimer -= dt;
            if (customer.madTimer <= 0.f)
                customer.state = CustomerState::Ordering;
            break;

        case CustomerState::Ordering:
            if (customer.showingMad) // just calmed (timeout or a correct item)
            {
                setCustomerFrame(assets, e, customer.spriteBase);
                customer.showingMad = false;
            }
            break;

        case CustomerState::Departing:
            break;
        }
    }
}

void customerCleanupSystem()
{
    static const bagel::Mask leavingMask =
        bagel::MaskBuilder().set<Leaving>().set<Customer>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(leavingMask)) continue;
        // Destroy only once the walk-out Tween has actually finished.
        if (e.get<Customer>().state != CustomerState::Departing || e.has<Tween>()) continue;
        e.addAll(Destroy{}); // destroySystem (end of frame) cascades to children (bubble already gone)
    }
}

} // namespace cafe
