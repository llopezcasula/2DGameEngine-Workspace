#include "Core/Engine.h"
#include "SpaceInvadersGame.h"
#include <iostream>

int main()
{
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════╗\n";
    std::cout << "║                                    ║\n";
    std::cout << "║       GATOR INVADERS               ║\n";
    std::cout << "║       UF Gators Edition            ║\n";
    std::cout << "║                                    ║\n";
    std::cout << "║    Destroy all invaders before     ║\n";
    std::cout << "║    they reach the bottom!          ║\n";
    std::cout << "║                                    ║\n";
    std::cout << "║    Go Gators! 🐊                   ║\n";
    std::cout << "║                                    ║\n";
    std::cout << "╚════════════════════════════════════╝\n";
    std::cout << "\n";

    // Create engine
    Engine engine;

    // Create Gator Invaders game
    SpaceInvadersGame game;

    // Run! The engine will call game.OnInit(), OnUpdate(), etc.
    engine.Run(&game);

    std::cout << "\nThanks for playing! Go Gators!\n";

    return 0;
}