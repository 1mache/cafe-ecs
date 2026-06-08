#include "CafeGame.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

int main()
{
    using namespace cafe;

    {
        std::ifstream  infoFile("res/info.json");
        nlohmann::json info;
        infoFile >> info;
        std::cout << "message: " << info["message"].get<std::string>() << "\n"
                  << "location: " << info["location"].get<std::string>()
                  << std::endl;
    }

    CafeGame game;
    game.init();
    game.run();

    return EXIT_SUCCESS;
}
