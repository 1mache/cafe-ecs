#pragma once
#include "Scene.h"

namespace cafe
{
/** @brief Boot menu: title text, white placeholder squares for the logo and the
 *  START / EXIT buttons, and a sound-toggle square (functional persistent mute).
 *  Placeholders are null-texture Drawables (solid tint quads) — swap in real art
 *  later by filling texture + srcRect, nothing else changes. START begins day 1,
 *  EXIT or window close quits. Mouse only. */
class StartMenuScene : public Scene
{
protected:
    void onInit() override;
    bool onUpdate(float dt) override;
    void onCleanup() override;
};
} // namespace cafe
