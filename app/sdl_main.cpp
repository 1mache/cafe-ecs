#include "CafeGame.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

int main()
{
    using namespace cafe;

    CafeGame game;
    game.init();
    game.run();
    game.destroy();

    return EXIT_SUCCESS;
}
