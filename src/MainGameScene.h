#pragma once
#include "Ingredient.h"
#include "PhysicsContext.h"
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
    PhysicsContext _physics{};

    static constexpr int  CUP_CAPACITY = 50;
    static constexpr auto BG_PATH = "bg.png";
};
}