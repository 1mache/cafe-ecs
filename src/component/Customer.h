#pragma once

#include "WorldPos.h"
#include <bagel.h>

namespace cafe
{
/** @brief General customer lifecycle state, driven by customerStateSystem.
 *  Arriving: walking in from CUSTOMER_ENTRANCE, no bubble/sensor yet.
 *  Ordering: seated, deliverable, bubble+talking anim attached.
 *  Mad: showing the rejected-item sprite (spriteBase + 2) until madTimer expires
 *       or a correct item calms it back to Ordering.
 *  Departing: bubble destroyed, walking out to CUSTOMER_EXIT; destroyed on arrival. */
enum class CustomerState { Arriving, Ordering, Mad, Departing };

struct Customer
{
    CustomerState state{ CustomerState::Arriving };
    int           spriteBase{};                   // this customer's first sheet frame:
                                                   // idle=base, talk=base/base+1, mad=base+2
    float         madTimer{ 0.f };                // seconds left showing the mad sprite
    bool          showingMad{ false };             // mad sprite currently pinned to Drawable?
    bagel::Entity bubble{ bagel::ent_type(-1) };  // order bubble child; set on arrival
};

// Movement / timing (tune by feel).
constexpr WorldPos CUSTOMER_ENTRANCE{ 12.f, 0.f };  // off right edge
constexpr WorldPos CUSTOMER_EXIT{ -12.f, 0.f };     // off left edge
constexpr float    CUSTOMER_WALK_IN_DURATION  = 1.0f;
constexpr float    CUSTOMER_WALK_OUT_DURATION = 1.5f;
constexpr float    CUSTOMER_MAD_TIME          = 1.0f;
} // namespace cafe

template <> struct bagel::Storage<cafe::Customer> final : NoInstance
{ using type = SparseStorage<cafe::Customer>; };
