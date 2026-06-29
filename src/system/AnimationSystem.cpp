#include "AnimationSystem.h"

#include "AssetManager.h"
#include "Components.h"
#include "Utils.h"
#include "bagel.h"

namespace cafe
{
namespace
{
// applies the relative transform from the animation to the objects transform
void applyRelativeTransform(Transform& t, Transform& relativeT)
{
    t.x += relativeT.x;
    t.y += relativeT.y;
    t.w *= relativeT.w;
    t.h *= relativeT.h;
    t.rot += relativeT.rot;
}
}

void animationSystem(AssetManager& assets, float dt)
{
    static auto animationMask = bagel::MaskBuilder().set<Animation>().set<Drawable>().set<Transform>().build();

    for (auto ent = bagel::Entity::first(); !ent.eof(); ent.next())
    {
        if (!ent.test(animationMask)) continue;

        auto& animation  = ent.get<Animation>();
        auto& t= ent.get<Transform>();
        auto& d = ent.get<Drawable>();

        animation.timer -= dt;
        if (animation.timer < 0)
        {
            auto frameIdOpt = getNextAnimationFrame(animation);
            if (!frameIdOpt.has_value())
            {
                ent.del<Animation>(); // animation has ended
                return;
            }
            int frameId = frameIdOpt.value();

            updateAnimationWithFrame(animation, frameId);
            AnimationFrame& frame = animation.frames[toSizet(frameId)];

            if (animation.hasSpriteSheet())
            {
                if (frame.spritesheetIndex < 0) fatalError("Spritesheet index -1 for frame even tho animation has spritesheet");
                d.srcRect = assets.getSpriteSheet(animation.spriteSheetName).getFrameRect(frame.spritesheetIndex);
            }

            applyRelativeTransform(t, frame.relativeTransform);
        }

    }
}
}