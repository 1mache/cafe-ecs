# add new code files here to compile
target_sources(
    ${EXECUTABLE_NAME}
    PRIVATE
    Systems.h
    Systems.cpp
    Entities.h
    Entities.cpp
    Components.h
    Utils.h

    GameConfig.h
    RenderContext.h
    
    TransformOperations.h
    TransformOperations.cpp
    WorldPos.h
)

target_include_directories(
    ${EXECUTABLE_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)