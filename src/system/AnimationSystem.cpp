#include "AnimationSystem.h"

#include "AssetManager.h"
#include "Components.h"
#include "Utils.h"
#include "bagel.h"

namespace cafe
{
void animationSystem(AssetManager& assets, float dt)
{
    static auto animationMask = bagel::MaskBuilder().set<Animation>().set<Drawable>().build();

    for (auto ent = bagel::Entity::first(); !ent.eof(); ent.next())
    {
        if (!ent.test(animationMask)) continue;

        auto& animation = ent.get<Animation>();
        auto& d = ent.get<Drawable>();

        animation.timer -= dt;
        if (animation.timer < 0)
        {
            auto frameIdOpt = getNextAnimationFrame(animation);
            if (!frameIdOpt.has_value())
            {
                ent.del<Animation>(); // animation has ended
                continue;
            }
            int frameId = frameIdOpt.value();

            updateAnimationWithFrame(animation, frameId);
            AnimationFrame& frame = animation.frames[toSizet(frameId)];

            if (frame.spritesheetIndex < 0) fatalError("Spritesheet index -1 for animation frame");
            d.srcRect = assets.getSpriteSheet(animation.spriteSheetName).getFrameRect(frame.spritesheetIndex);
        }
    }
}
}