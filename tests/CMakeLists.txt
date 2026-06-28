add_executable(unit_tests
    test_day_state.cpp
    test_day_report.cpp
    test_glyph.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/DayReport.cpp
)

target_include_directories(unit_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(unit_tests PRIVATE Catch2::Catch2WithMain)

include(Catch)
catch_discover_tests(unit_tests)
