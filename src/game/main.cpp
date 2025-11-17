#include "core/Application.h"

int main ()
{
    auto game = core::Application("res", core::WindowState(1280, 720, "td_game", false));

    game.run();
}
