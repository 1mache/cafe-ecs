#pragma once
#include "Ingredient.h"
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
    bagel::Entity _inputEnt{ static_cast<bagel::ent_type>(-1) };
    // One pour pipe per ingredient (indexed by Ingredient).
    bagel::Entity _pipes[INGREDIENT_COUNT]{
        bagel::Entity{ static_cast<bagel::ent_type>(-1) },
        bagel::Entity{ static_cast<bagel::ent_type>(-1) },
        bagel::Entity{ static_cast<bagel::ent_type>(-1) },
    };
    bool          _isDragging{false};

    static constexpr int  CUP_CAPACITY = 50;
    static constexpr auto BG_PATH = "bg.png";
};
}