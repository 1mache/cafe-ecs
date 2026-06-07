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
    component/Leaving.h
    component/Order.h
    component/SpeechAnchor.h
    component/Components.h

    # systems
    system/RenderSystem.h
    system/RenderSystem.cpp
    system/TransformSystem.h
    system/TransformSystem.cpp
    system/CoffeeSystem.h
    system/CoffeeSystem.cpp
    system/ClientSystem.h
    system/ClientSystem.cpp
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
    entity/ClientFactory.h
    entity/ClientFactory.cpp
    entity/SpeechBubbleFactory.h
    entity/SpeechBubbleFactory.cpp
    entity/OrderIconFactory.h
    entity/OrderIconFactory.cpp
    entity/Entities.h
    entity/Entities.cpp

    # root helpers
    GameConfig.h
    RenderContext.h
    PhysicsFilters.h
    PhysicsContext.h
    PhysicsContext.cpp

    WorldPos.h

    Assets.h
    Assets.cpp
    AssetManager.h
    AssetManager.cpp
    Texture.h
    Texture.cpp
    Utils.h
)

target_include_directories(
    ${EXECUTABLE_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/component
    ${CMAKE_CURRENT_SOURCE_DIR}/system
    ${CMAKE_CURRENT_SOURCE_DIR}/entity
)
