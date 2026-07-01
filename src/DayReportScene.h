#pragma once
#include "Scene.h"

namespace cafe
{
/** @brief End-of-day receipt + upgrade shop. Banks the day's score into the
 *  persistent wallet, draws the report rows and world-space shop buttons, lets
 *  the player buy upgrades, and starts the next day on NEXT DAY / Enter / Space. */
class DayReportScene : public Scene
{
protected:
    void onInit() override;
    bool onUpdate(float dt) override;
    void onCleanup() override;
};
} // namespace cafe
