# set(APP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/main.cc")
set(APP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/sdl_main.cpp")

add_executable(${EXECUTABLE_NAME} ${APP_SOURCES})

target_link_libraries(
    ${EXECUTABLE_NAME}
    PRIVATE "LibFoo"
            "LibBar"
            SDL3-static
            SDL3_image-static
            nlohmann_json::nlohmann_json
            spdlog::spdlog
            cxxopts::cxxopts)

if(${ENABLE_WARNINGS})
    target_set_warnings(
        TARGET
        ${EXECUTABLE_NAME}
        ENABLE
        ${ENABLE_WARNINGS}
        AS_ERRORS
        ${ENABLE_WARNINGS_AS_ERRORS})
endif()

if(${ENABLE_LTO})
    target_enable_lto(
        TARGET
        ${EXECUTABLE_NAME}
        ENABLE
        ON)
endif()

if(${ENABLE_CLANG_TIDY})
    add_clang_tidy_to_target(${EXECUTABLE_NAME})
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(${EXECUTABLE_NAME} PRIVATE -Wno-old-style-cast)
endif()
