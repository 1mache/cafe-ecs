#include "CustomerSystem.h"
#include "Components.h"
#include "Entities.h"
#include "MainGameScene.h"
#include "Menu.h"
#include <bagel.h>
#include <cmath>
#include <iostream>
#include <vector>

namespace cafe
{
// TODO: remove after change to dynamic patience
static constexpr float CUSTOMER_PATIENCE = 60.f; // seconds before a customer leaves unhappy


void customerSpawnerSystem(AssetManager& assets, PhysicsContext& physics, float dtSeconds)
{
    static const bagel::Mask spawnerMask =
        bagel::MaskBuilder().set<CustomerSpawner>().build();
    static const bagel::Mask customerMask =
        bagel::MaskBuilder().set<Order>().set<Behavior>().build();

    // The seat is "occupied" while any customer exists.
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
            spawnCustomer(assets, physics, sp.seat, randomOrder(), CUSTOMER_PATIENCE);
            sp.cooldown = sp.interval;
        }
    }
}

void behaviorSystem(float dtSeconds)
{
    static const bagel::Mask behaviorMask =
        bagel::MaskBuilder().set<Behavior>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(behaviorMask)) continue;

        auto& behavior = e.get<Behavior>();
        behavior.patience -= dtSeconds;

        if (behavior.patience <= 0.f && !e.has<Leaving>())
            e.add(Leaving{});
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

void reportLeavingCustomers()
{
    static const bagel::Mask leavingMask =
        bagel::MaskBuilder().set<Leaving>().set<Behavior>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(leavingMask)) continue;

        const auto& behavior = e.get<Behavior>();
        if (behavior.succeeded)
            std::cout << "[Order] Customer left SUCCESSFUL — rating: " << behavior.rating << "\n";
        else
            std::cout << "[Order] Customer left FAILED (patience ran out) — rating: " << behavior.rating << "\n";
    }
}

void customerCleanupSystem()
{
    static const bagel::Mask leavingMask =
        bagel::MaskBuilder().set<Leaving>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(leavingMask)) continue;
        destroyPhysicalEntity(e.entity());
    }
}

} // namespace cafe
