#pragma once

namespace cafe
{
/** @brief Process-global day-cycle state. Survives scene swaps (the ECS world is
 *  torn down between scenes, so cross-day data cannot live in an entity).
 *  Modeled on RenderContext: a static service, no instances. */
class DayState
{
public:
    static int  dayNumber() { return _dayNumber; }
    static int  served()    { return _served; }
    static int  lost()      { return _lost; }
    static int  score()     { return _score; }

    static void beginNewDay()
    {
        ++_dayNumber;
        _served = 0;
        _lost   = 0;
        _score  = 0;
    }

    static void record(bool succeeded, int rating)
    {
        if (succeeded) ++_served;
        else           ++_lost;
        _score += rating;
    }

    static void resetAll()
    {
        _dayNumber = 0;
        _served    = 0;
        _lost      = 0;
        _score     = 0;
    }

private:
    static inline int _dayNumber{ 0 };
    static inline int _served{ 0 };
    static inline int _lost{ 0 };
    static inline int _score{ 0 };
};
} // namespace cafe
