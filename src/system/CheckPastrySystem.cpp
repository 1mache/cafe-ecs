#include "CheckPastrySystem.h"
#include "Components.h"
#include "Entities.h"
#include <vector>

namespace cafe
{
void checkPastrySystem()
{
    std::vector<bagel::ent_type> toDestroy;

    static const bagel::Mask mask =
        bagel::MaskBuilder().set<Pastry>().set<CheckPastryIntent>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask)) continue;

        const auto& intent = e.get<CheckPastryIntent>();
        bagel::Entity customer{ intent.customer };

        // Guard: customer may have left already — free the pastry's intent and destroy it.
        if (!customer.has<Order>() || !customer.has<OrderGrade>())
        {
            e.del<CheckPastryIntent>();
            toDestroy.push_back(e.entity());
            continue;
        }

        const int slot = intent.pastrySlot;
        auto& grade    = customer.get<OrderGrade>();

        // Wrong pastry type: bounce it back so the player can correct the order.
        if (e.get<Pastry>().type != intent.type)
        {
            if (e.has<DragIntent>())
                e.get<DragIntent>().dropSpaceEntity = std::nullopt;
            e.del<CheckPastryIntent>();
            rejectItem(e);
            continue;
        }

        const bool wantHot = intent.temp == Temperature::Hot;
        const bool isHot   = e.get<Pastry>().temperature == HEATED_TEMPERATURE;
        grade.pastryGrades[slot] = (wantHot == isHot) ? MAX_ITEM_GRADE : PARTIAL_ITEM_GRADE;
        markPastryServed(grade, slot);

        e.del<CheckPastryIntent>();
        toDestroy.push_back(e.entity());
    }

    // Destroy accepted (or orphaned) pastries after the iteration is complete.
    for (auto id : toDestroy)
        destroyDeliveredItem(id);
}
} // namespace cafe
