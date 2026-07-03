#include <catch2/catch_test_macros.hpp>
#include <bagel.h>

// Root-cause reproduction for the "cup/pastry/customer vanish mid-game" bug.
//
// bagel entity ids are bare, REUSED indices with NO generation counter
// (bagel.h: `struct ent_type { id_type id; };`, and createEntity() pops a freed
// index straight off the freelist). So any stored entity reference
// (ChildOf.parent, Liquid.holdingContainer, CheckCoffeeIntent.customer, ...)
// becomes a dangling index the moment its target is destroyed, and silently
// retargets to whatever new entity reuses that index. destroyDeliveredItem()
// (Entities.cpp) then destroys victims purely by `storedRef.id == target.id`,
// so a reused index can cascade a destroy into unrelated entities.
//
// These tests pin that mechanism deterministically, with no timing luck. They
// use tiny local stand-in components so the test needs only bagel.h (the real
// components drag in box2d/SDL, which unit_tests does not link).

namespace reuse_test
{
struct Customer {};   // stands in for a real customer entity
struct Cup {};        // stands in for a real cup entity
// Mirrors ChildOf.parent / Liquid.holdingContainer: a component that stores a
// raw entity index across frames.
struct Ref { bagel::ent_type target{ bagel::ent_type{ -1 } }; };
} // namespace reuse_test

template <> struct bagel::Storage<reuse_test::Customer> final : NoInstance { using type = TaggedStorage<reuse_test::Customer>; };
template <> struct bagel::Storage<reuse_test::Cup>      final : NoInstance { using type = TaggedStorage<reuse_test::Cup>; };
template <> struct bagel::Storage<reuse_test::Ref>      final : NoInstance { using type = SparseStorage<reuse_test::Ref>; };

using namespace reuse_test;

TEST_CASE("bagel hands a destroyed entity's index straight back (no generation)")
{
    auto      a   = bagel::Entity::create();
    const int aId = a.entity().id;
    a.add(Customer{});
    REQUIRE(a.has<Customer>());

    a.destroy();                        // frees index aId

    auto b = bagel::Entity::create();   // reuses it (freelist is LIFO)
    REQUIRE(b.entity().id == aId);
    b.add(Cup{});

    // A reference captured before the destroy still holds the raw index aId.
    // With no generation to tell old from new, it now resolves to the NEW entity.
    bagel::Entity staleRef{ bagel::ent_type{ aId } };
    REQUIRE(staleRef.has<Cup>());             // stale ref points at the new cup...
    REQUIRE_FALSE(staleRef.has<Customer>());  // ...not the destroyed customer

    b.destroy();
}

TEST_CASE("destroy-by-index sweep hits an unrelated entity after index reuse")
{
    // Mirrors destroyDeliveredItem (Entities.cpp): for a cup it collects every
    // entity whose stored reference `.id == cup.id`. Here a child stores a
    // reference to a customer; the customer leaves; a fresh cup reuses the freed
    // index; the cup's destroy sweep now matches the child that never belonged
    // to it -> the mass-deletion smoking gun.

    auto customer = bagel::Entity::create();
    customer.add(Customer{});
    const int customerId = customer.entity().id;

    auto child = bagel::Entity::create();
    child.add(Ref{ .target = bagel::ent_type{ customerId } });

    customer.destroy();                       // frees customerId

    auto cup = bagel::Entity::create();       // reuses customerId (LIFO)
    cup.add(Cup{});
    const int cupId = cup.entity().id;
    REQUIRE(cupId == customerId);

    // The exact predicate destroyDeliveredItem uses to collect victims.
    static const bagel::Mask refMask = bagel::MaskBuilder().set<Ref>().build();
    bool childWouldBeDestroyed = false;
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(refMask)) continue;
        if (e.get<Ref>().target.id == cupId)
            childWouldBeDestroyed = true;
    }

    // BUG: the child never belonged to this cup, yet it matches by index and
    // would be swept into the cup's destroy.
    REQUIRE(childWouldBeDestroyed);

    child.destroy();
    cup.destroy();
}
