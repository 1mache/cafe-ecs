#pragma once
#include "Scene.h"
#include "bagel.h"

namespace cafe
{
class MainGameScene : public Scene
{
protected:
    void onInit() override;
    bool onUpdate(float dt) override;
    void onCleanup() override;

private:
    bagel::Entity _machineEnt{static_cast<bagel::ent_type>(-1)};
    bagel::Entity _inputEnt{static_cast<bagel::ent_type>(-1)};

    static constexpr int  CUP_CAPACITY = 50;
    static constexpr auto BG_PATH = "bg.png";
};
}