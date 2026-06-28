#pragma once
#include "Scene.h"

namespace cafe
{
/** @brief End-of-day receipt screen. Reads DayState, draws the rows, and on any
 *  click/key transitions to a fresh MainGameScene (next day). */
class DayReportScene : public Scene
{
protected:
    void onInit() override;
    bool onUpdate(float dt) override;
    void onCleanup() override;
};
} // namespace cafe
