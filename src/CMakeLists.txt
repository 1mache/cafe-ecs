# add new code files here to compile
target_sources(
    ${EXECUTABLE_NAME}
    PRIVATE
    example.cpp
    example.h
)

target_include_directories(
    ${EXECUTABLE_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
