#pragma once

namespace cafe
{
/** @brief Identifies which scene to run next. Returned by Scene::run(). */
enum class SceneId { StartMenu, MainGame, DayReport, HowToPlay, Quit };
} // namespace cafe
