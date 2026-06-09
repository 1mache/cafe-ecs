#include "CafeGame.h"

int main()
{
    using namespace cafe;
    CafeGame game;
    game.init();
    game.run();
    game.destroy();

    return EXIT_SUCCESS;
}