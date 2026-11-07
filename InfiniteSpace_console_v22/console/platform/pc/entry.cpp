#include "engine.h"

int main() {
    Engine engine;
    Game* game = create_game();
    engine.run(*game);
    delete game;
    return 0;
}
