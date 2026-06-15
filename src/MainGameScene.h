#pragma once
#include "Ingredient.h"
#include "Scene.h"
#include "WorldPos.h"
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

    static constexpr float CUSTOMER_PATIENCE = 60.f; // seconds before a customer leaves unhappy
    static constexpr float SPAWN_INTERVAL    = 2.f;  // seconds between one customer leaving and the next
    static constexpr auto  BG_PATH = "bg.png";
    // Supply slot/button layout lives in SupplySystem.h (namespace cafe::supply).
};
}