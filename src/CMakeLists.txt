# add new code files here to compile
target_sources(
    ${EXECUTABLE_NAME}
    PRIVATE
    # components
    component/Transform.h
    component/Transform.cpp
    component/Drawable.h
    component/ChildOf.h
    component/Liquid.h
    component/PhysicsBody.h
    component/Cup.h
    component/CoffeeSpawner.h
    component/CleanupZone.h
    component/Behavior.h
    component/Draggable.h
    component/Draggable.cpp
    component/DropSpace.h
    component/DropType.h
    component/Held.h
    component/Leaving.h
    component/Order.h
    component/Served.h
    component/Components.h

    # systems
    system/RenderSystem.h
    system/RenderSystem.cpp
    system/TransformSystem.h
    system/TransformSystem.cpp
    system/CoffeeSystem.h
    system/CoffeeSystem.cpp
    system/CustomerSystem.h
    system/CustomerSystem.cpp
    system/DragAndDropSystem.h
    system/DragAndDropSystem.cpp
    system/InputSystem.h
    system/Systems.h

    # entities
    entity/CupFactory.h
    entity/CupFactory.cpp
    entity/LiquidDropFactory.h
    entity/LiquidDropFactory.cpp
    entity/CoffeeMachineFactory.h
    entity/CoffeeMachineFactory.cpp
    entity/CleanupZoneFactory.h
    entity/CleanupZoneFactory.cpp
    entity/CustomerFactory.h
    entity/CustomerFactory.cpp
    entity/SpeechBubbleFactory.h
    entity/SpeechBubbleFactory.cpp
    entity/OrderIconFactory.h
    entity/OrderIconFactory.cpp
    entity/CafeEnvironmentFactory.h
    entity/CafeEnvironmentFactory.cpp
    entity/PastryFactory.h
    entity/PastryFactory.cpp
    entity/Entities.h
    entity/Entities.cpp

    # root helpers
    GameConfig.h
    Utils.h
    RenderContext.h

    PhysicsFilters.h
    PhysicsContext.h
    PhysicsContext.cpp

    WorldPos.h

    Assets.h
    Assets.cpp

    Scene.h
    Scene.cpp
    AssetManager.h
    AssetManager.cpp
    Texture.h
    Texture.cpp
    CafeGame.h
    CafeGame.cpp
)

target_include_directories(
    ${EXECUTABLE_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/component
    ${CMAKE_CURRENT_SOURCE_DIR}/system
    ${CMAKE_CURRENT_SOURCE_DIR}/entity
)
