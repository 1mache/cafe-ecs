// Benchmark: pre-rework per-frame gather+sort (commit 51e2db1 "dummy drawable
// layer sorting") vs the current persistent sorted-cache ordering
// (src/system/RenderSystem.cpp drawSystem). Measures ONLY the ordering work —
// the per-sprite GPU draw loop is identical in both versions.
//
// Test entry point (see CLAUDE.md): enable by pointing APP_SOURCES at this file
// in app/CMakeLists.txt. Remove before merging to main.
#include "Components.h" // Drawable, Transform
#include <bagel.h>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <random>
#include <vector>

using namespace cafe;
using Entity = bagel::Entity;

namespace
{
constexpr int BASE_ENTITIES   = 400;  // static scene size (~real game scale)
constexpr int CHURN_DROPS     = 120;  // live "liquid drops" during churn
constexpr int CHURN_PER_FRAME = 30;   // drops destroyed + spawned each frame
constexpr int WARMUP_FRAMES   = 200;
constexpr int TIMED_FRAMES    = 2000;
constexpr unsigned SEED       = 42;   // fixed: both impls see identical scenes

const bagel::Mask drawMask =
    bagel::MaskBuilder().set<Drawable>().set<Transform>().build();

layer::RenderLayer randomLayer(std::mt19937& rng)
{
    std::uniform_int_distribution<int> dist(0, static_cast<int>(layer::COUNT) - 1);
    return static_cast<layer::RenderLayer>(dist(rng));
}

Entity spawnDrawable(layer::RenderLayer l)
{
    auto e = Entity::create();
    e.addAll(
        Transform{ .x = 0.f, .y = 0.f, .w = 1.f, .h = 1.f },
        Drawable{ nullptr, SDL_FRect{}, l });
    return e;
}

// Position-weighted checksum: proves both impls produce the same layer order
// and stops the optimizer from discarding the work.
long long checksumOf(const Entity* ents, int n)
{
    long long sum = 0;
    for (int i = 0; i < n; ++i)
        sum += static_cast<long long>(i + 1)
             * static_cast<long long>(ents[i].get<Drawable>().renderLayer);
    return sum;
}

// --- OLD (51e2db1): fresh vector + full sort, every frame ---------------------
long long orderOld()
{
    std::vector<Entity> drawables{};
    for (auto e = Entity::first(); !e.eof(); e.next())
        if (e.test(drawMask))
            drawables.push_back(e);

    std::sort(drawables.begin(), drawables.end(),
              [](const Entity& a, const Entity& b)
              { return a.get<Drawable>().renderLayer < b.get<Drawable>().renderLayer; });

    return checksumOf(drawables.data(), static_cast<int>(drawables.size()));
}

// --- NEW (newSortedRedner branch tip) and FIX (current drawSystem) ------------
// Both keep a persistent cache + membership bag + dirty bit; they differ in the
// re-sort strategy:
//   stepSort=true  -> branch tip (3b77aec): ONE bubble pass per dirty frame,
//                     dirty stays set until a pass finds nothing to swap.
//                     (Debug couts from that commit omitted; the malloc-init fix
//                     is kept in both so the bench itself isn't UB.)
//   stepSort=false -> current (b89cb2b): full std::stable_sort on dirty frames.
// State lives in a struct so each scenario run starts from a fresh cache.
struct SortedCache
{
    bool stepSort = false;
    bagel::Bag<Entity, bagel::InitialEntities> sorted{};
    bagel::Bag<bool, bagel::InitialEntities>   inSorted{};
    bagel::id_type                             initializedUpTo = 0;
    bool                                       dirty = false;

    void remove(Entity e)
    {
        const bagel::id_type id = e.entity().id;
        for (int i = 0; i < sorted.size(); ++i)
        {
            if (sorted[i].entity().id != id)
                continue;
            sorted[i] = sorted[sorted.size() - 1];
            sorted.pop();
            return;
        }
    }

    long long order()
    {
        for (auto e = Entity::first(); !e.eof(); e.next())
        {
            inSorted.ensure(e.entity().id + 1);
            while (initializedUpTo <= e.entity().id)
            {
                inSorted[initializedUpTo] = false;
                ++initializedUpTo;
            }
            const bool hasDrawable = e.test(drawMask);
            const bool cached      = inSorted[e.entity().id];

            if (hasDrawable && !cached)
            {
                sorted.push(e);
                inSorted[e.entity().id] = true;
                dirty = true;
            }
            else if (!hasDrawable && cached)
            {
                remove(e);
                inSorted[e.entity().id] = false;
                dirty = true;
            }
        }

        if (dirty && sorted.size() > 1)
        {
            if (stepSort)
            {
                // Branch tip: one bubble pass; clear dirty only once fully sorted.
                bool isSorted = true;
                for (int i = 0; i < sorted.size() - 1; ++i)
                {
                    if (sorted[i].get<Drawable>().renderLayer >
                        sorted[i + 1].get<Drawable>().renderLayer)
                    {
                        isSorted = false;
                        std::swap(sorted[i], sorted[i + 1]);
                    }
                }
                if (isSorted)
                    dirty = false;
            }
            else
            {
                std::stable_sort(
                    &sorted[0], &sorted[0] + sorted.size(),
                    [](const Entity& a, const Entity& b)
                    { return a.get<Drawable>().renderLayer < b.get<Drawable>().renderLayer; });
                dirty = false;
            }
        }
        else
        {
            dirty = false;
        }

        return checksumOf(&sorted[0], sorted.size());
    }
};

// --- Scene + scenario driver ---------------------------------------------------
void destroyEverything()
{
    for (auto e = Entity::first(); !e.eof(); e.next())
        e.destroy();
}

struct Stats
{
    double    avgUs{};
    double    maxUs{};
    long long checksum{};
};

// Runs one impl over one scenario. `orderFn` returns the frame's checksum.
template <class OrderFn>
Stats runScenario(bool churn, OrderFn orderFn)
{
    std::mt19937 rng{ SEED };

    for (int i = 0; i < BASE_ENTITIES; ++i)
        spawnDrawable(randomLayer(rng));

    // Churn entities get mixed layers, like real play: LIQUID drops + UI2 FX
    // particles + popup text spawning/dying together across layers.
    std::vector<Entity> drops{};
    if (churn)
        for (int i = 0; i < CHURN_DROPS; ++i)
            drops.push_back(spawnDrawable(randomLayer(rng)));

    Stats  s{};
    double totalUs = 0.0;
    size_t oldest  = 0;

    for (int frame = 0; frame < WARMUP_FRAMES + TIMED_FRAMES; ++frame)
    {
        if (churn)
        {
            // Spawn BEFORE destroying: recycled ids then come from entities the
            // caches already dropped last frame, so every add/remove is visible
            // as a membership change. (Same-frame destroy->spawn id reuse would
            // hide the change from BOTH cache variants — a separate, latent
            // issue that would muddy this comparison.)
            for (int i = 0; i < CHURN_PER_FRAME; ++i)
            {
                const Entity fresh = spawnDrawable(randomLayer(rng));
                drops[oldest].destroy();
                drops[oldest] = fresh;
                oldest        = (oldest + 1) % drops.size();
            }
        }

        const auto t0 = std::chrono::steady_clock::now();
        s.checksum    = orderFn();
        const auto t1 = std::chrono::steady_clock::now();

        if (frame < WARMUP_FRAMES)
            continue;
        const double us =
            std::chrono::duration<double, std::micro>(t1 - t0).count();
        totalUs += us;
        s.maxUs = std::max(s.maxUs, us);
    }

    s.avgUs = totalUs / TIMED_FRAMES;
    destroyEverything();
    return s;
}
} // namespace

int main()
{
    std::printf("render-sort bench: %d base entities, churn %d/frame of %d drops, "
                "%d timed frames (+%d warmup)\n\n",
                BASE_ENTITIES, CHURN_PER_FRAME, CHURN_DROPS, TIMED_FRAMES, WARMUP_FRAMES);
    std::printf("%-28s %-8s %12s %12s %16s\n",
                "impl", "scene", "avg us/frame", "max us/frame", "checksum");

    for (const bool churn : { false, true })
    {
        const char* scene = churn ? "churn" : "static";

        const Stats old = runScenario(churn, [] { return orderOld(); });
        std::printf("%-28s %-8s %12.2f %12.2f %16lld\n",
                    "old: gather+sort each frame", scene,
                    old.avgUs, old.maxUs, old.checksum);

        {
            SortedCache cache{ .stepSort = true };
            const Stats s = runScenario(churn, [&] { return cache.order(); });
            std::printf("%-28s %-8s %12.2f %12.2f %16lld%s\n",
                        "new: cache + step sort", scene, s.avgUs, s.maxUs, s.checksum,
                        s.checksum != old.checksum ? "  <- WRONG ORDER" : "");
        }

        {
            SortedCache cache{};
            const Stats s = runScenario(churn, [&] { return cache.order(); });
            std::printf("%-28s %-8s %12.2f %12.2f %16lld%s\n",
                        "fix: cache + full sort", scene, s.avgUs, s.maxUs, s.checksum,
                        s.checksum != old.checksum ? "  <- WRONG ORDER" : "");
        }
        std::printf("\n");
    }

    return 0;
}