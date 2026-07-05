#include <catch2/catch_test_macros.hpp>
#include <bagel.h>

// Root-cause reproduction for the "cup/pastry/customer vanish mid-game" bug,
// and a regression test for destroySystem's fix.
//
// bagel entity ids are bare, REUSED indices with NO generation counter
// (bagel.h: `struct ent_type { id_type id; };`, and createEntity() pops a freed
// index straight off the freelist). So any stored entity reference
// (ChildOf.parent, Liquid.holdingContainer, CheckCoffeeIntent.customer, ...)
// becomes a dangling index the moment its target is destroyed, and silently
// retargets to whatever new entity reuses that index.
//
// destroySystem (DestroySystem.cpp) avoids this by never destroying anything
// while it's still discovering dependents: it tags a Destroy marker, closes
// over ChildOf/holdingContainer references by checking whether the CURRENT
// owner at that index has the tag, and only calls the real destroy once the
// whole closure is computed. So an id can never be freed and reused mid-pass,
// and a stale reference to an unrelated (non-tagged) entity that happens to
// now occupy a reused index is correctly ignored.
//
// These tests pin both properties deterministically, with no timing luck.
// They use tiny local stand-in components so the test needs only bagel.h (the
// real components drag in box2d/SDL, which unit_tests does not link).

namespace reuse_test
{
struct Customer {};  // stands in for a real customer entity
struct Cup {};       // stands in for a real cup entity
struct Destroy {};   // stands in for cafe::Destroy
// Mirrors ChildOf.parent / Liquid.holdingContainer: a component that stores a
// raw entity index across frames.
struct Ref { bagel::ent_type target{ bagel::ent_type{ -1 } }; };
} // namespace reuse_test

template <> struct bagel::Storage<reuse_test::Customer> final : NoInstance { using type = TaggedStorage<reuse_test::Customer>; };
template <> struct bagel::Storage<reuse_test::Cup>      final : NoInstance { using type = TaggedStorage<reuse_test::Cup>; };
template <> struct bagel::Storage<reuse_test::Destroy>  final : NoInstance { using type = TaggedStorage<reuse_test::Destroy>; };
template <> struct bagel::Storage<reuse_test::Ref>      final : NoInstance { using type = SparseStorage<reuse_test::Ref>; };

using namespace reuse_test;

namespace
{
// Mirrors destroySystem's closure: tag anything whose Ref.target currently
// has Destroy, looping to a fixpoint. Never destroys anything itself.
void closeOverRefs()
{
    static const bagel::Mask refMask     = bagel::MaskBuilder().set<Ref>().build();
    static const bagel::Mask destroyMask = bagel::MaskBuilder().set<Destroy>().build();

    bool changed = true;
    while (changed)
    {
        changed = false;
        for (auto e = bagel::Entity::first(); !e.eof(); e.next())
        {
            if (e.test(destroyMask)) continue;
            if (!e.test(refMask)) continue;

            const auto target = e.get<Ref>().target;
            if (target.id < 0) continue;
            if (!bagel::Entity{ target }.has<Destroy>()) continue;

            e.addAll(Destroy{});
            changed = true;
        }
    }
}
} // namespace

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

TEST_CASE("destroySystem closure tags a real dependent, ignores a stale ref to a reused index")
{
    // Real dependent: child.Ref points at a parent tagged Destroy this frame.
    auto parent = bagel::Entity::create();
    parent.add(Destroy{});

    auto child = bagel::Entity::create();
    child.add(Ref{ .target = parent.entity() });

    // Stale dependent: some OTHER, unrelated entity from a past "frame" was
    // destroyed and its index got reused by a fresh entity that is NOT tagged
    // Destroy. A leftover Ref to that old index must not falsely cascade.
    auto longGone = bagel::Entity::create();
    const int reusedId = longGone.entity().id;
    longGone.destroy();

    auto freshOwner = bagel::Entity::create(); // reuses reusedId (LIFO)
    REQUIRE(freshOwner.entity().id == reusedId);
    freshOwner.add(Cup{});                      // alive, unrelated, not tagged Destroy

    auto staleChild = bagel::Entity::create();
    staleChild.add(Ref{ .target = bagel::ent_type{ reusedId } });

    closeOverRefs();

    REQUIRE(child.has<Destroy>());              // real dependent: cascaded
    REQUIRE_FALSE(staleChild.has<Destroy>());    // stale ref: correctly ignored
    REQUIRE(freshOwner.has<Cup>());              // untouched by the closure

    parent.destroy();
    child.destroy();
    freshOwner.destroy();
    staleChild.destroy();
}
