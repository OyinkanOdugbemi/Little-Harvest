#include <exception>
#include <iostream>

#include "Game.hpp"

int main()
{
    try
    {
        Game game;
        game.run();
    }
    catch (const std::exception& error)
    {
        std::cerr << "Little Harvest crashed: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
