#include "Systems.h"
#include "Components.h"
#include <catch2/catch_test_macros.hpp>
#include <vector>
#include <bagel.h>

using namespace cafe;

TEST_CASE("coffeeSpawnerSystem emits at the configured interval",
          "[spawner]")
{
    // Create a single spawner entity.
    auto ent = bagel::Entity::create();
    ent.addAll(
        Transform{ .x = 0.f, .y = 0.f, .w = 0.5f, .h = 0.5f },
        CoffeeSpawner{ .interval = 0.1f, .accumulator = 0.f,
                       .active = true, .offset = {0.f, 0.f} });

    std::vector<WorldPos> drops;
    auto capture = [&drops](WorldPos p) { drops.push_back(p); };

    // 10 ticks of dt = 0.05 s → total simulated time = 0.5 s
    // Interval = 0.1 s → expect 5 drops.
    for (int i = 0; i < 10; ++i)
        coffeeSpawnerSystemImpl(0.05f, capture);

    REQUIRE(drops.size() == 5);
    for (const auto& d : drops) {
        REQUIRE(d.x == 0.f);
        REQUIRE(d.y == 0.f);
    }

    ent.destroy();
}

TEST_CASE("coffeeSpawnerSystem does not emit while inactive",
          "[spawner]")
{
    auto ent = bagel::Entity::create();
    ent.addAll(
        Transform{ .x = 1.f, .y = 2.f, .w = 0.5f, .h = 0.5f },
        CoffeeSpawner{ .interval = 0.05f, .accumulator = 0.f,
                       .active = false, .offset = {0.f, 0.f} });

    std::vector<WorldPos> drops;
    coffeeSpawnerSystemImpl(1.0f, [&](WorldPos p){ drops.push_back(p); });
    REQUIRE(drops.empty());

    ent.destroy();
}

TEST_CASE("coffeeSpawnerSystem applies spawn offset", "[spawner]")
{
    auto ent = bagel::Entity::create();
    ent.addAll(
        Transform{ .x = 3.f, .y = 4.f, .w = 0.5f, .h = 0.5f },
        CoffeeSpawner{ .interval = 0.1f, .accumulator = 0.f,
                       .active = true, .offset = { 1.f, -0.5f } });

    std::vector<WorldPos> drops;
    coffeeSpawnerSystemImpl(0.15f, [&](WorldPos p){ drops.push_back(p); });

    REQUIRE(drops.size() == 1);
    REQUIRE(drops[0].x == 4.0f);   // 3 + 1
    REQUIRE(drops[0].y == 3.5f);   // 4 + (-0.5)

    ent.destroy();
}
