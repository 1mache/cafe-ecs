#pragma once
#include "Scene.h"

namespace cafe
{
/** @brief How-to-play screen: black background (how-to content will be white text
 *  drawn on it, added later) and a small BACK button in the bottom-left corner
 *  that returns to the start menu. The BACK button is a null-texture Drawable
 *  (solid tint quad) — swap in real art later by filling texture + srcRect,
 *  nothing else changes. Mouse only. */
class HowToPlayScene : public Scene
{
protected:
    void onInit() override;
    bool onUpdate(float dt) override;
    void onCleanup() override;
};
} // namespace cafe
