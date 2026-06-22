#include "OrderGrade.h"

namespace cafe
{

int firstUnservedDrink(const Order& order, const OrderGrade& grade)
{
    for (int i = 0; i < order.drinkCount; ++i)
        if (!isDrinkServed(grade, i)) return i;
    return -1;
}

int firstUnservedPastry(const Order& order, const OrderGrade& grade)
{
    for (int j = 0; j < order.pastryCount; ++j)
        if (!isPastryServed(grade, j)) return j;
    return -1;
}

bool allItemsServed(const Order& order, const OrderGrade& grade)
{
    for (int i = 0; i < order.drinkCount; ++i)
        if (!isDrinkServed(grade, i)) return false;
    for (int j = 0; j < order.pastryCount; ++j)
        if (!isPastryServed(grade, j)) return false;
    return true;
}

int sumItemGrades(const Order& order, const OrderGrade& grade)
{
    int total = 0;
    for (int i = 0; i < order.drinkCount; ++i)
        total += grade.drinkGrades[i];
    for (int j = 0; j < order.pastryCount; ++j)
        total += grade.pastryGrades[j];
    return total;
}

} // namespace cafe
