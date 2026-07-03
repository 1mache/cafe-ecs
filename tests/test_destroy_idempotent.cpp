#include <catch2/catch_test_macros.hpp>
#include <bagel.h>

#include "Cup.h"
#include "Entities.h"

// Regression test for the "machines/pastries/cup vanish after day 1" bug.
//
// bagel's World::deleteEntity pushes the freed id onto its free-list
// unconditionally, so destroying an entity TWICE puts its id on the list
// twice — and createEntity then hands the SAME id to two live entities.
// They share one mask and one component slot, so destroying either erases
// both. destroyAllGameEntities() used to trigger exactly that every scene
// teardown by re-destroying ids already freed during the day.
//
// destroyPhysicalEntity now guards on an empty mask (= already destroyed),
// making every destroy path idempotent. This pins that guarantee.

TEST_CASE("destroyPhysicalEntity is idempotent: a double destroy never hands one id to two entities")
{
    auto e = bagel::Entity::create();
    e.add(cafe::Cup{});
    const int id = e.entity().id;

    cafe::destroyPhysicalEntity(e.entity());
    cafe::destroyPhysicalEntity(e.entity()); // must be a no-op, not a second free

    // The freed id is on top of the LIFO free-list exactly once: the first
    // create gets it back, the second must get a different id.
    auto a = bagel::Entity::create();
    auto b = bagel::Entity::create();
    REQUIRE(a.entity().id == id);
    REQUIRE(b.entity().id != id);

    // a and b are distinct entities: components on one never leak to the other.
    a.add(cafe::Cup{});
    REQUIRE_FALSE(b.has<cafe::Cup>());

    cafe::destroyPhysicalEntity(a.entity());
    b.destroy();
}
