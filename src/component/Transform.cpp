#include "Transform.h"
#include "GameConfig.h"

namespace cafe
{
SDL_FPoint worldToScreenPoint(WorldPos worldPos, WorldPos camPos)
{
    const float s     = SCALE_FACTOR;
    const float centx = LOGICAL_W * 0.5f;
    const float centy = LOGICAL_H * 0.5f;

    // Camera world coord -> screen center. Y-up world -> Y-down screen.
    return SDL_FPoint{centx + (worldPos.x - camPos.x) * PTM * s,
                      centy - (worldPos.y - camPos.y) * PTM * s};
}

WorldPos screenToWorldPoint(SDL_FPoint screenPos, WorldPos camPos)
{
    const float s     = SCALE_FACTOR;
    const float centx = LOGICAL_W * 0.5f;
    const float centy = LOGICAL_H * 0.5f;

    return WorldPos{camPos.x + (screenPos.x - centx) / (PTM * s),
                    camPos.y - (screenPos.y - centy) / (PTM * s)};
}

bool isPointInsideTransform(const WorldPos& p, const Transform& t)
{
    return p.x > t.x - t.w && p.x < t.x + t.w &&
           p.y > t.y - t.h && p.y < t.y + t.h;
}

SDL_FRect transformToFrect(const Transform& t, WorldPos camPos)
{
    // Y-up world: top-left of AABB is (x - w, y + h).
    const auto topLeft = worldToScreenPoint({t.x - t.w, t.y + t.h}, camPos);

    return SDL_FRect{.x = topLeft.x,
                     .y = topLeft.y,
                     .w = worldToScreenScale(t.w),
                     .h = worldToScreenScale(t.h)};
}

b2Vec2 transformToB2Pos(const Transform& t)
{
    return b2Vec2{t.x, t.y};
}

b2Vec2 transformToB2Scale(const Transform& t)
{
    return b2Vec2{t.w, t.h};
}

void transformUpdateWithB2Pos(Transform& t, b2Vec2 pos)
{
    t.x = pos.x;
    t.y = pos.y;
}

} // namespace cafe
