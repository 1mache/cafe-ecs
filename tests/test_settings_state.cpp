#include <catch2/catch_test_macros.hpp>
#include "SettingsState.h"

using namespace cafe;

TEST_CASE("sound is on by default")
{
    SettingsState::resetAll();
    REQUIRE(SettingsState::muted() == false);
}

TEST_CASE("toggleMuted flips the flag both ways")
{
    SettingsState::resetAll();
    SettingsState::toggleMuted();
    REQUIRE(SettingsState::muted() == true);
    SettingsState::toggleMuted();
    REQUIRE(SettingsState::muted() == false);
}
