add_executable(unit_tests
    test_day_state.cpp
    test_day_report.cpp
    test_destroy_idempotent.cpp
    test_entity_reuse.cpp
    test_glyph.cpp
    test_grade_tier.cpp
    test_order_match.cpp
    test_upgrade_catalog.cpp
    test_upgrade_state.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/DayReport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/OrderMatch.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/UpgradeCatalog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/entity/Entities.cpp
)

target_include_directories(unit_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/component
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/entity
)

target_include_directories(unit_tests SYSTEM PRIVATE
    ${CMAKE_SOURCE_DIR}/dependencies/bagel
)

target_link_libraries(unit_tests PRIVATE Catch2::Catch2WithMain SDL3-static box2d)

include(Catch)
catch_discover_tests(unit_tests)
