#include "engine.h"

#include "core/Application.h"

#include <fmt/format.h>
#include <formatting/fmt_asset.h>

// static_assert(fmt::is_formattable_v<core::assets::AssetId>);

using namespace core;

int main(int argc, char** argv)
{
    Application game = Application(argc, argv, "res", engine::initial_window_state());

    // Game hook!!1
    engine::register_flecs(game.getEcsWorld());

    game.run();
}