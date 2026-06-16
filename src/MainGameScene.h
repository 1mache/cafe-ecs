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

    static constexpr float CUSTOMER_PATIENCE = 60.f; // seconds before a customer leaves unhappy
    static constexpr float SPAWN_INTERVAL    = 2.f;  // seconds between one customer leaving and the next
    static constexpr auto  BG_PATH = "bg.png";
    // Supply slot/button layout lives in SupplySystem.h (namespace cafe::supply).
};
}