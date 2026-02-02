#include "Core/Engine.h"
#include "GatorInvaders.h"
#include <iostream>

#ifdef _WIN32
    #include <windows.h>
#endif


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
    GatorInvaders game;

    // Run! The engine will call game.OnInit(), OnUpdate(), etc.
    engine.Run(&game);

    std::cout << "\nThanks for playing! Go Gators!\n";

    return 0;
}

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    return main();
}
#endif
