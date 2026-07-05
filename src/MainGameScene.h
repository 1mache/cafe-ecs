#pragma once
#include "PhysicsContext.h"
#include "Scene.h"

namespace cafe
{
class MainGameScene : public Scene
{
protected:
    void onInit() override;
    bool onUpdate(float dt) override;
    void onCleanup() override;

private:
    PhysicsContext _physics{};

    static constexpr int   CUSTOMERS_PER_DAY = 1;    // customers (served+lost) per day
    static constexpr float SPAWN_INTERVAL    = 2.f;  // seconds between one customer leaving and the next
    static constexpr auto  BG_PATH = "bg_big.png";
    // Supply slot/button layout lives in Supply.h (namespace cafe::supply).
};
}