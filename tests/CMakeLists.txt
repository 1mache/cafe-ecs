add_executable(unit_tests
    test_day_state.cpp
    test_day_report.cpp
    test_glyph.cpp
    test_order_match.cpp
    test_upgrade_catalog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/DayReport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/OrderMatch.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/UpgradeCatalog.cpp
)

target_include_directories(unit_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/component
)

target_include_directories(unit_tests SYSTEM PRIVATE
    ${CMAKE_SOURCE_DIR}/dependencies/bagel
)

target_link_libraries(unit_tests PRIVATE Catch2::Catch2WithMain SDL3-static)

include(Catch)
catch_discover_tests(unit_tests)
