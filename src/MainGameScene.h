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

    static constexpr float DAY_LENGTH        = 120.f; // seconds per day
    static constexpr float SPAWN_INTERVAL    = 2.f;  // seconds between one customer leaving and the next
    static constexpr auto  BG_PATH = "bg.png";
    // Supply slot/button layout lives in SupplySystem.h (namespace cafe::supply).
};
}