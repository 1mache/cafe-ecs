#include <catch2/catch_test_macros.hpp>
#include <bagel.h>

// Regression tests for PackedStorage::del swap-and-pop correctness.
// The old implementation wrote out-of-bounds when deleting the last element and
// left _idToComp[ent.id] stale after any delete — both caused overlapping-tween
// corruption when TweenSystem held a reference across del().

namespace packed_del_test
{
struct Payload
{
    int value{};
};
} // namespace packed_del_test

template <>
struct bagel::Storage<packed_del_test::Payload> final : NoInstance
{
    using type = PackedStorage<packed_del_test::Payload>;
};

using namespace packed_del_test;

TEST_CASE("PackedStorage del swap case preserves survivor data and remaps index")
{
    auto a = bagel::Entity::create();
    auto b = bagel::Entity::create();
    a.add(Payload{ .value = 1 });
    b.add(Payload{ .value = 2 });

    a.del<Payload>();

    REQUIRE_FALSE(a.has<Payload>());
    REQUIRE(b.has<Payload>());
    REQUIRE(b.get<Payload>().value == 2);
}

TEST_CASE("PackedStorage del last element does not write out of bounds")
{
    auto only = bagel::Entity::create();
    only.add(Payload{ .value = 42 });

    only.del<Payload>();

    REQUIRE_FALSE(only.has<Payload>());
}

TEST_CASE("PackedStorage del first of two leaves survivor readable via get")
{
    auto first  = bagel::Entity::create();
    auto second = bagel::Entity::create();
    first.add(Payload{ .value = 10 });
    second.add(Payload{ .value = 20 });

    first.del<Payload>();

    REQUIRE(second.get<Payload>().value == 20);
    REQUIRE_FALSE(first.has<Payload>());
}
