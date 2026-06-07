#pragma once

#include "SDL3/SDL_rect.h"
#include <bagel.h>

namespace cafe
{

/** @brief Offset from an entity's center to its "speech" point (e.g. mouth),
 *  in logical pixels, Y-up. Used to anchor a speech bubble's tail. */
struct SpeechAnchor
{
    SDL_FPoint mouthOffset{};
};

} // namespace cafe

template <>
struct bagel::Storage<cafe::SpeechAnchor> final : NoInstance
{
    using type = SparseStorage<cafe::SpeechAnchor>;
};
