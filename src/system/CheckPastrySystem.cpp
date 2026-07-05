#include "CheckPastrySystem.h"
#include "Components.h"
#include "CustomerSystem.h"
#include "Entities.h"

namespace cafe
{
void checkPastrySystem()
{
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
            e.addAll(Destroy{});
            continue;
        }

        const Pastry& pastry = e.get<Pastry>();
        const Order&  order  = customer.get<Order>();
        auto&         grade  = customer.get<OrderGrade>();
        const int     slot   = matchPastrySlot(order, grade, pastry.type, pastry.temperature);

        if (slot < 0)
        {
            if (e.has<DragIntent>())
                e.get<DragIntent>().dropSpaceEntity = std::nullopt;
            e.del<CheckPastryIntent>();
            rejectItem(e);
            makeCustomerMad(customer);
            continue;
        }

        const bool wantHot = order.pastries[slot].temp == Temperature::Hot;
        const bool isHot   = pastry.temperature == Temperature::Hot;
        grade.pastryGrades[slot] = (wantHot == isHot) ? MAX_ITEM_GRADE : PARTIAL_ITEM_GRADE;
        markPastryServed(grade, slot);
        calmCustomer(customer);

        e.del<CheckPastryIntent>();
        e.addAll(Destroy{});
    }
}
} // namespace cafe
