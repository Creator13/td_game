#include "engine.h"

#include <tracy/Tracy.hpp>

#include "core/Application.h"

using namespace core;

int main(int argc, char** argv)
{
    assets::AssetDatabase::initialize("res");
    // TODO this scope is a temporary solution to the problem where an Application always
    //  needs to be constructed after AssetDB gets built, and destroyed before
    //  the DB is destroyed. It is functional but find a nicer solution to this.
    //  Possible solution is a global, self-managed singleton from a base singleton class
    {
        Application game = Application(argc, argv, engine::getInitialWindowState());

        // Game hook!!
        {
            ZoneScopedN("Game setup")
            engine::setupGame(game.getEcsWorld());
        }

        // Game loop
        game.run();
    }
    assets::AssetDatabase::destroy();
}
