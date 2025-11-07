#include "core/Application.h"

int main ()
{
    auto game = core::Application::init("res", core::WindowState(1280, 720, "td_game", false));

    if (!game) return -1;

    game.value()->run();
}
