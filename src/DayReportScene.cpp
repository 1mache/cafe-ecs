#include "DayReportScene.h"

#include "DayReport.h"
#include "GameConfig.h"
#include "Glyph.h"
#include "Text.h"
#include "Texture.h"
#include <SDL3/SDL.h>

namespace cafe
{
namespace
{
constexpr auto  FONT_TEX = "font.png";
constexpr int   SCALE    = 1;
constexpr float MARGIN_X = 24.f;
constexpr float TOP_Y    = 12.f;
constexpr float LINE_H   = static_cast<float>((GLYPH_H + 3) * SCALE);
} // namespace

void DayReportScene::onInit()
{
    getAssetManager().getTexture(FONT_TEX); // warm the cache
}

bool DayReportScene::onUpdate(float /*dt*/)
{
    SDL_Renderer* renderer = getRenderer();

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            requestNext(SceneId::Quit);
            return false;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_KEY_DOWN:
            requestNext(SceneId::MainGame); // start the next day
            return false;
        default:
            break;
        }
    }

    SDL_SetRenderDrawColor(renderer, 30, 20, 16, 255);
    SDL_RenderClear(renderer);

    const Texture& font = getAssetManager().getTexture(FONT_TEX);
    const auto     rows = buildReportRows();

    float y = TOP_Y;
    for (const auto& row : rows)
    {
        if (!row.label.empty())
            drawText(renderer, font, row.label, MARGIN_X, y, SCALE);
        if (!row.value.empty())
        {
            const float vw = textWidth(row.value, SCALE);
            drawText(renderer, font, row.value,
                     static_cast<float>(LOGICAL_W) - MARGIN_X - vw, y, SCALE);
        }
        y += LINE_H;
    }

    SDL_RenderPresent(renderer);
    return true;
}

void DayReportScene::onCleanup() {}
} // namespace cafe
