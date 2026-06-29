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

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(dragMask)) continue;

        auto& intent = e.get<DragIntent>();
        if (intent.intentType != DragIntentType::released) continue;
        if (!intent.dropSpaceEntity.has_value()) continue;

        bagel::Entity target{ *intent.dropSpaceEntity };

        if (!target.has<Order>() || !target.has<OrderGrade>()) continue;

        const auto& order = target.get<Order>();
        const auto& grade = target.get<OrderGrade>();

        if (e.has<Cup>())
        {
            // Find the first unserved drink slot; grade via acceptGradedBeverageSystem.
            const int slot = firstUnservedDrink(order, grade);
            if (slot < 0 || e.has<CheckCoffeeIntent>())
            {
                rejectItem(e);
                intent.dropSpaceEntity = std::nullopt;
                continue;
            }

            const auto& recipe = recipeFor(order.drinks[slot].type);
            CheckCoffeeIntent coffeeIntent{};
            for (size_t i = 0; i < INGREDIENT_COUNT; ++i)
                coffeeIntent.ratio[i] = recipe.ratio[i];
            coffeeIntent.targetFill = recipe.targetFill;
            coffeeIntent.isHot     = order.drinks[slot].temp == Temperature::Hot;
            coffeeIntent.customer  = target.entity();
            coffeeIntent.drinkSlot = slot;
            e.add(coffeeIntent);
        }
        else // pastry
        {
            const int slot = firstUnservedPastry(order, grade);
            if (slot < 0 || e.has<CheckPastryIntent>())
            {
                rejectItem(e);
                intent.dropSpaceEntity = std::nullopt;
                continue;
            }

            CheckPastryIntent pastryIntent{};
            pastryIntent.type       = order.pastries[slot].type;
            pastryIntent.temp       = order.pastries[slot].temp;
            pastryIntent.customer   = target.entity();
            pastryIntent.pastrySlot = slot;
            e.add(pastryIntent);
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
