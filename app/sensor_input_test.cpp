#include "Components.h"
#include "GameConfig.h"
#include "PhysicsContext.h"
#include "PhysicsFilters.h"
#include "RenderContext.h"
#include "Scene.h"
#include "Systems.h"
#include "Transform.h"
#include "Utils.h"
#include <SDL3/SDL.h>
#include <box2d/box2d.h>
#include <cstdlib>
#include <cstdint>
#include <iostream>

namespace
{
constexpr cafe::WorldPos kSensorPos{ 0.f, 0.f };
constexpr float          kSensorHalfW = 2.f;
constexpr float          kSensorHalfH = 1.25f;
constexpr float          kVisitorHalf = 0.35f;

b2BodyId createBodyForEntity(bagel::Entity ent,
                             b2BodyType bodyType,
                             cafe::WorldPos pos,
                             float gravityScale)
{
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = bodyType;
    bodyDef.position = { pos.x, pos.y };
    bodyDef.gravityScale = gravityScale;
    bodyDef.userData = reinterpret_cast<void*>(static_cast<uintptr_t>(ent.entity().id));
    return b2CreateBody(cafe::PhysicsContext::world(), &bodyDef);
}

bagel::Entity createSensorEntity()
{
    auto ent = bagel::Entity::create();
    const b2BodyId body = createBodyForEntity(ent, b2_staticBody, kSensorPos, 0.f);

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.isSensor = true;
    shapeDef.enableSensorEvents = false;
    shapeDef.filter.categoryBits = cafe::filter::DROPSPACE_SENSOR;
    shapeDef.filter.maskBits = cafe::filter::MASK_DROPSPACE_SENSOR;

    b2Polygon box = b2MakeOffsetBox(kSensorHalfW, kSensorHalfH, { 0.f, 0.f }, b2Rot_identity);
    b2CreatePolygonShape(body, &shapeDef, &box);

    ent.addAll(cafe::Transform{ .x = kSensorPos.x,
                                .y = kSensorPos.y,
                                .w = kSensorHalfW,
                                .h = kSensorHalfH },
               cafe::PhysicsBody{ body },
               cafe::DropSpace{ .dropType = cafe::DropType::Any });
    cafe::enableSensor(ent.entity());
    return ent;
}

bagel::Entity createVisitorEntity(cafe::WorldPos pos)
{
    auto ent = bagel::Entity::create();
    const b2BodyId body = createBodyForEntity(ent, b2_dynamicBody, pos, 0.f);

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.isSensor = true;
    shapeDef.enableSensorEvents = true;
    shapeDef.filter.categoryBits = cafe::filter::DRAGGABLE;
    shapeDef.filter.maskBits = cafe::filter::MASK_DRAGGABLE;

    b2Polygon box = b2MakeOffsetBox(kVisitorHalf, kVisitorHalf, { 0.f, 0.f }, b2Rot_identity);
    b2CreatePolygonShape(body, &shapeDef, &box);

    ent.addAll(cafe::Transform{ .x = pos.x,
                                .y = pos.y,
                                .w = kVisitorHalf,
                                .h = kVisitorHalf },
               cafe::PhysicsBody{ body },
               cafe::DragIntent{ .dropType = cafe::DropType::Any });
    return ent;
}

void moveVisitor(bagel::Entity visitor, cafe::WorldPos pos)
{
    auto& transform = visitor.get<cafe::Transform>();
    transform.x = pos.x;
    transform.y = pos.y;

    const b2BodyId body = visitor.get<cafe::PhysicsBody>().id;
    if (!b2Body_IsValid(body)) return;

    b2Body_SetTransform(body, { pos.x, pos.y }, b2Rot_identity);
    b2Body_SetLinearVelocity(body, { 0.f, 0.f });
}

void syncVisitorTransform(bagel::Entity visitor)
{
    const b2BodyId body = visitor.get<cafe::PhysicsBody>().id;
    if (!b2Body_IsValid(body)) return;

    const b2Vec2 pos = b2Body_GetPosition(body);
    auto& transform = visitor.get<cafe::Transform>();
    transform.x = pos.x;
    transform.y = pos.y;
}

void renderBox(SDL_Renderer* renderer, const cafe::Transform& transform, SDL_Color color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_FRect rect = cafe::transformToFrect(transform, cafe::RenderContext::getCameraPos());
    SDL_RenderFillRect(renderer, &rect);
}

void logTriggeredControls(const cafe::SdlEvents& input)
{
    using cafe::Controls;
    const auto isTriggered = [&input](Controls control)
    {
        return (input.controls & (1u << static_cast<int>(control))) != 0u;
    };

    if (isTriggered(Controls::Quit))
        std::cout << "[SensorInputTest] Control: Quit\n";

    if (isTriggered(Controls::KeyDown))
    {
        std::cout << "[SensorInputTest] Control: KeyDown scancode="
                  << SDL_GetScancodeName(input.keyScancode) << "\n";
    }

    if (isTriggered(Controls::KeyUp))
    {
        std::cout << "[SensorInputTest] Control: KeyUp scancode="
                  << SDL_GetScancodeName(input.keyScancode) << "\n";
    }

    if (isTriggered(Controls::MouseButtonDown))
    {
        std::cout << "[SensorInputTest] Control: MouseButtonDown button="
                  << static_cast<int>(input.mouseButton)
                  << " pos=(" << input.mousePos.x << ", " << input.mousePos.y << ")\n";
    }

    if (isTriggered(Controls::MouseButtonUp))
    {
        std::cout << "[SensorInputTest] Control: MouseButtonUp button="
                  << static_cast<int>(input.mouseButton)
                  << " pos=(" << input.mousePos.x << ", " << input.mousePos.y << ")\n";
    }
}

// Mirrors MainGameScene: a Scene subclass that publishes input through
// intentSystem (IntentSystem.cpp) instead of polling SDL locally.
class SensorInputTestScene final : public cafe::Scene
{
protected:
    void onInit() override
    {
        using namespace cafe;
        PhysicsContext::init();

        // Single input-state entity: intentSystem polls SDL and publishes here.
        _inputEnt = bagel::Entity::create();
        _inputEnt.add(SdlEvents{});

        _sensorEnt  = createSensorEntity();
        _visitorEnt = createVisitorEntity({ -4.f, 0.f });

        std::cout << "[SensorInputTest] Move the mouse to move the visitor.\n"
                  << "[SensorInputTest] E enables the sensor, D disables it, Esc quits.\n"
                  << "[SensorInputTest] Triggered controls are logged each frame.\n"
                  << "[SensorInputTest] Temporary test entry point; remove before merging to main.\n";
    }

    bool onUpdate(float dt) override
    {
        using namespace cafe;
        auto* renderer = getRenderer();

        // Single SDL poll: publishes SdlEvents to the input-state entity.
        intentSystem(renderer);
        const auto& input = _inputEnt.get<SdlEvents>();
        logTriggeredControls(input);

        if (isTriggeredEvent(_inputEnt.entity(), Controls::Quit))
            return false;

        if (isTriggeredEvent(_inputEnt.entity(), Controls::KeyDown))
        {
            if (input.keyScancode == SDL_SCANCODE_ESCAPE)
            {
                return false;
            }
            else if (input.keyScancode == SDL_SCANCODE_E && !_sensorEnabled)
            {
                enableSensor(_sensorEnt.entity());
                _sensorEnabled = true;
            }
            else if (input.keyScancode == SDL_SCANCODE_D && _sensorEnabled)
            {
                disableSensor(_sensorEnt.entity());
                _sensorEnabled = false;
            }
        }

        // Drag-and-drop: intentSystem set the per-entity DragIntent transitions
        // above; this moves the held visitor to the cursor and snaps/drops it on
        // release. dropSpaceDetectionSystem (after the step) reads sensor events.
        dragAndDropSystem();

        PhysicsContext::step(dt);
        sensorGatheringSystem();
        dropSpaceDetectionSystem();
        syncVisitorTransform(_visitorEnt);

        const bool isOverlapping = _visitorEnt.has<InSensorArea>();
        const int sensorId = isOverlapping ? _visitorEnt.get<InSensorArea>().sensor.id : -1;
        if (isOverlapping != _wasOverlapping || sensorId != _lastSensorId)
        {
            if (isOverlapping)
            {
                std::cout << "[SensorInputTest] Visitor entered Sensor entity="
                          << sensorId << "\n";
            }
            else
            {
                std::cout << "[SensorInputTest] Visitor left sensor area\n";
            }
            _wasOverlapping = isOverlapping;
            _lastSensorId   = sensorId;
        }

        SDL_SetRenderDrawColor(renderer, 20, 20, 24, 255);
        SDL_RenderClear(renderer);
        renderBox(renderer,
                  _sensorEnt.get<Transform>(),
                  _sensorEnabled ? SDL_Color{ 70, 130, 180, 140 }
                                 : SDL_Color{ 60, 60, 60, 140 });
        renderBox(renderer,
                  _visitorEnt.get<Transform>(),
                  isOverlapping ? SDL_Color{ 80, 220, 120, 255 }
                                : SDL_Color{ 220, 180, 80, 255 });
        SDL_RenderPresent(renderer);

        return true;
    }

    void onCleanup() override
    {
        cafe::PhysicsContext::shutdown();
        std::cout << "[SensorInputTest] Ended\n";
    }

private:
    bagel::Entity _inputEnt{ static_cast<bagel::ent_type>(-1) };
    bagel::Entity _sensorEnt{ static_cast<bagel::ent_type>(-1) };
    bagel::Entity _visitorEnt{ static_cast<bagel::ent_type>(-1) };

    bool _sensorEnabled{ true };
    bool _wasOverlapping{ false };
    int  _lastSensorId{ -1 };
};
} // namespace

int main()
{
    using namespace cafe;

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "Init error : " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }

    SDL_Window* window{};
    SDL_Renderer* renderer{};
    SDL_CreateWindowAndRenderer("SDL Input and Sensor Test",
                                static_cast<int>(START_WIN_W),
                                static_cast<int>(START_WIN_H),
                                SDL_WINDOW_OPENGL,
                                &window,
                                &renderer);
    assertFatal(window != nullptr, SDL_GetError());
    assertFatal(renderer != nullptr, SDL_GetError());

    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    RenderContext::init(window, renderer);

    SensorInputTestScene scene;
    scene.init(renderer);
    scene.run();
    scene.cleanup();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
