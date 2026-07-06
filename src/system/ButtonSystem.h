#pragma once

#include "WorldPos.h"
#include <SDL3/SDL.h>

namespace cafe
{
class AssetManager;
class AudioContext;
class PhysicsContext;

/** @brief Single shared hit-test for every Button entity (Menu/Shop/Sound/Spawn/
 *  Machine), replacing the four copy-pasted loops that used to live in
 *  updateMachineButtonIntent, updateSpawnButtonIntent, menuInputSystem and
 *  shopInputSystem. Callers poll SDL themselves (one poller per scene, per
 *  docs/code-conventions.md) and pass the already-computed world-space mouse
 *  position plus this frame's down/up edges.
 *
 *  Momentary kinds (Menu/Shop/Sound/Spawn): sets `pressed` on a down-click that
 *  lands inside the Transform. Left for the owning consumer to clear.
 *  Machine (held): sets `pressed` on a down-click inside the Transform, clears it
 *  on any mouse-up (mirrors the old updateMachineButtonIntent behaviour). */
void updateButtonsFromMouse(WorldPos worldMouse, bool clicked, bool mouseUp, AudioContext& audio);

/** @brief One-shot consume of every pressed Button that isn't Menu or Machine
 *  (those are handled by the owning scene's Menu read and machineButtonSystem
 *  respectively): Shop buys the upgrade, Sound flips mute, Spawn (skipping
 *  Pastry, owned by pastrySupplySystem) spawns a cup/ice cube. Folds the former
 *  shopPurchaseSystem and supplyButtonSystem, plus the sound-toggle action.
 *
 *  `physics` is nullable: only the Spawn case needs it (to build the spawned
 *  entity's body), and only MainGameScene ever has Spawn buttons + a
 *  PhysicsContext. StartMenuScene/DayReportScene (Sound/Shop only) pass
 *  nullptr. Asserts if a Spawn button is ever pressed with no physics given.
 *
 *  Also refreshes live sprite-state for buttons that need it regardless of
 *  `pressed` (which is already consumed above by the time drawSystem runs):
 *  Button{kind==Spawn, dropType==Cup} shows ui_buttons.json's "cup" tag frame 1
 *  while the mouse is held down over it (frame 0 otherwise) — real-time
 *  mouse-held state, not the momentary/consumed `pressed` edge, so the feedback
 *  is visible for the whole hold. Button{kind==Sound} shows "soundon"/"soundoff"
 *  per SettingsState::muted(). Button{kind==Menu} (Start/Exit) shows
 *  ui_buttons2.png frame 1 while held over it, same as Cup. */
void buttonDispatchSystem(AssetManager& assets, PhysicsContext* physics, AudioContext& audio,
                          SDL_Renderer* renderer);
} // namespace cafe
