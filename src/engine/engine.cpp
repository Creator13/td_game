#include "engine.h"

#include "core/Application.h"

using namespace core;

int main(int argc, char** argv)
{
    Application game = Application(argc, argv, "res", engine::initial_window_state());

    // Game hook!!
    engine::register_flecs(game.getEcsWorld());

    // Game loop
    game.run();
}