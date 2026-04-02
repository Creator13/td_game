#include "engine.h"

#include "core/Application.h"

using namespace core;

int main(int argc, char** argv)
{
    Application game = Application(argc, argv, "res", engine::getInitialWindowState());

    // Game hook!!
    engine::setupGame(game.getEcsWorld());

    // Game loop
    game.run();
}