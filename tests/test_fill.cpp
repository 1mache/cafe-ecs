#include "Components.h"
#include "Entities.h"
#include "PhysicsContext.h"
#include "Systems.h"
#include <bagel.h>
#include <box2d/box2d.h>
#include <catch2/catch_test_macros.hpp>

using namespace cafe;

// ----- Small helpers used by several test cases -----

// Steps physics + drains sensor events for `frames` ticks at 1/60 s each.
static void stepFor(int frames)
{
    for (int i = 0; i < frames; ++i)
    {
        PhysicsContext::step(1.f / 60.f);
        sensorEventSystem();
    }
}

// Counts every entity currently carrying a Liquid tag.
static int countLiquidEntities()
{
    static const bagel::Mask liquidMask = bagel::MaskBuilder().set<Liquid>().build();
    int n = 0;
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
        if (e.test(liquidMask)) ++n;
    return n;
}

// ----- Tests -----

TEST_CASE("A coffee drop falls into the cup, fills it by 1, and is destroyed", "[fill]")
{
    PhysicsContext::init();

    // Cup 4 m x 3 m at the origin; drop spawned 5 m above it.
    // With GRAVITY=25, a 4 m fall takes ~0.566 s — 60 fixed steps (1 s) is plenty.
    auto cupEnt = createCupHeadless({ 0.f, 0.f }, 32.f, 24.f, 50);
    createLiquidDrop({ 0.f, 5.f });

    stepFor(60);

    REQUIRE(cupEnt.get<Cup>().filled == 1);
    REQUIRE(countLiquidEntities() == 0);

    destroyPhysicalEntity(cupEnt.entity());
    PhysicsContext::shutdown();
}

TEST_CASE("Cup at capacity: drop is still destroyed, counter does not overflow", "[fill]")
{
    PhysicsContext::init();

    // Capacity = 1 — first drop fills it, second drop tests the saturating branch.
    auto cupEnt = createCupHeadless({ 0.f, 0.f }, 32.f, 24.f, 1);
    createLiquidDrop({ 0.f, 5.f });
    stepFor(60);
    REQUIRE(cupEnt.get<Cup>().filled == 1);
    REQUIRE(cupEnt.get<Cup>().isFull());

    // Second drop into the now-full cup.
    createLiquidDrop({ 0.f, 5.f });
    stepFor(60);
    REQUIRE(cupEnt.get<Cup>().filled == 1);     // did NOT exceed capacity
    REQUIRE(countLiquidEntities() == 0);        // drop was still destroyed

    destroyPhysicalEntity(cupEnt.entity());
    PhysicsContext::shutdown();
}

TEST_CASE("A drop that misses the cup is destroyed by the cleanup zone", "[fill]")
{
    PhysicsContext::init();
    auto cleanupEnt = createCleanupZone();

    // Spawn high up; no cup in the world, so the drop will fall past where the
    // cup would be and into the cleanup sensor (centered at y = -8).
    createLiquidDrop({ 0.f, 5.f });
    REQUIRE(countLiquidEntities() == 1);

    // 2 s of simulated time is enough to fall ~50 m under gravity = 25.
    stepFor(120);
    REQUIRE(countLiquidEntities() == 0);

    destroyPhysicalEntity(cleanupEnt.entity());
    PhysicsContext::shutdown();
}

TEST_CASE("Two independent cups have independent fill counters", "[fill]")
{
    PhysicsContext::init();

    auto cupA = createCupHeadless({ -5.f, 0.f }, 32.f, 24.f, 50);
    auto cupB = createCupHeadless({  5.f, 0.f }, 32.f, 24.f, 50);
    createLiquidDrop({ -5.f, 5.f });
    createLiquidDrop({  5.f, 5.f });

    stepFor(60);

    REQUIRE(cupA.get<Cup>().filled == 1);
    REQUIRE(cupB.get<Cup>().filled == 1);
    REQUIRE(countLiquidEntities() == 0);

    destroyPhysicalEntity(cupA.entity());
    destroyPhysicalEntity(cupB.entity());
    PhysicsContext::shutdown();
}

TEST_CASE("Two drops into one cup: both counted, both destroyed", "[fill]")
{
    PhysicsContext::init();
    auto cupEnt = createCupHeadless({ 0.f, 0.f }, 32.f, 24.f, 50);

    // Spawn the second drop a bit higher so they don't collide mid-air.
    createLiquidDrop({ 0.f, 5.f });
    createLiquidDrop({ 0.f, 6.f });

    stepFor(60);

    REQUIRE(cupEnt.get<Cup>().filled == 2);
    REQUIRE(countLiquidEntities() == 0);

    destroyPhysicalEntity(cupEnt.entity());
    PhysicsContext::shutdown();
}

TEST_CASE("destroyPhysicalEntity actually destroys the b2Body", "[entities]")
{
    PhysicsContext::init();
    auto cupEnt = createCupHeadless({ 0.f, 0.f }, 32.f, 24.f, 50);
    const b2BodyId body = cupEnt.get<PhysicsBody>().id;

    REQUIRE(b2Body_IsValid(body));
    destroyPhysicalEntity(cupEnt.entity());
    REQUIRE_FALSE(b2Body_IsValid(body));

    PhysicsContext::shutdown();
}

TEST_CASE("Cup::fillPercent and Cup::isFull behave correctly at edges", "[components]")
{
    // No physics needed for this — pure component logic.
    REQUIRE(Cup{}.fillPercent() == 0.f);           // capacity = 0 → 0, no div-by-zero
    REQUIRE(Cup{}.isFull());                       // capacity = 0 → already full
    REQUIRE(Cup{ 10, 0  }.fillPercent() == 0.f);
    REQUIRE(Cup{ 10, 5  }.fillPercent() == 0.5f);
    REQUIRE(Cup{ 10, 10 }.fillPercent() == 1.f);
    REQUIRE(Cup{ 10, 10 }.isFull());
    REQUIRE_FALSE(Cup{ 10, 9 }.isFull());
}
