#pragma once

#include <cstdint>

namespace core::time
{
    void markFrame();
    void init();

    float delta();
    uint64_t ticks();
    float sinceLoad();

    float onePercent();
    float averageDelta();
    float fps();
}
