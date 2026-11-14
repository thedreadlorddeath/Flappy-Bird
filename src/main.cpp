#include "Game.hpp"
#include <ctime>
#include <cstdlib>

#if defined(__ANDROID__)
#define MAIN_EXPORT __attribute__((visibility("default")))
#else
#define MAIN_EXPORT
#endif

MAIN_EXPORT int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    // Seed pseudo-random number generator
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    Game game;
    game.run();

    return 0;
}
