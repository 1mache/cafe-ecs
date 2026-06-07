include(Catch)

add_executable(UnitTestFilters test_filters.cpp)
target_link_libraries(UnitTestFilters PRIVATE Catch2::Catch2WithMain)
target_include_directories(UnitTestFilters PRIVATE "${CMAKE_SOURCE_DIR}/src")

if(${ENABLE_WARNINGS})
    target_set_warnings(
        TARGET UnitTestFilters
        ENABLE ${ENABLE_WARNINGS}
        AS_ERRORS ${ENABLE_WARNINGS_AS_ERRORS})
endif()

get_filename_component(_compiler_bin_dir "${CMAKE_CXX_COMPILER}" DIRECTORY)
catch_discover_tests(UnitTestFilters
    DISCOVERY_MODE PRE_TEST
    DL_PATHS "${_compiler_bin_dir}")

add_executable(UnitTestSpawner
    test_spawner.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems.cpp
    ${CMAKE_SOURCE_DIR}/src/TransformOperations.cpp
    ${CMAKE_SOURCE_DIR}/src/Entities.cpp
    ${CMAKE_SOURCE_DIR}/src/Assets.cpp
    ${CMAKE_SOURCE_DIR}/src/PhysicsContext.cpp)
target_link_libraries(UnitTestSpawner
    PRIVATE
        Catch2::Catch2WithMain
        SDL3-static
        SDL3_image-static
        box2d)
target_include_directories(UnitTestSpawner
    SYSTEM PRIVATE
        "${CMAKE_SOURCE_DIR}/dependencies/bagel")
target_include_directories(UnitTestSpawner
    PRIVATE
        "${CMAKE_SOURCE_DIR}/src")

if(${ENABLE_WARNINGS})
    target_set_warnings(
        TARGET UnitTestSpawner
        ENABLE ${ENABLE_WARNINGS}
        AS_ERRORS ${ENABLE_WARNINGS_AS_ERRORS})
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(UnitTestSpawner PRIVATE -Wno-old-style-cast)
endif()

catch_discover_tests(UnitTestSpawner
    DISCOVERY_MODE PRE_TEST
    DL_PATHS "${_compiler_bin_dir}")

add_executable(UnitTestFill
    test_fill.cpp
    ${CMAKE_SOURCE_DIR}/src/Systems.cpp
    ${CMAKE_SOURCE_DIR}/src/TransformOperations.cpp
    ${CMAKE_SOURCE_DIR}/src/Entities.cpp
    ${CMAKE_SOURCE_DIR}/src/PhysicsContext.cpp
    ${CMAKE_SOURCE_DIR}/src/Assets.cpp)
target_link_libraries(UnitTestFill
    PRIVATE
        Catch2::Catch2WithMain
        SDL3-static
        SDL3_image-static
        box2d)
target_include_directories(UnitTestFill
    SYSTEM PRIVATE
        "${CMAKE_SOURCE_DIR}/dependencies/bagel")
target_include_directories(UnitTestFill
    PRIVATE
        "${CMAKE_SOURCE_DIR}/src")

if(${ENABLE_WARNINGS})
    target_set_warnings(
        TARGET UnitTestFill
        ENABLE ${ENABLE_WARNINGS}
        AS_ERRORS ${ENABLE_WARNINGS_AS_ERRORS})
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(UnitTestFill PRIVATE -Wno-old-style-cast)
endif()

catch_discover_tests(UnitTestFill
    DISCOVERY_MODE PRE_TEST
    DL_PATHS "${_compiler_bin_dir}")
