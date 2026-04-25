#pragma once

#include "datatype.h"

namespace core::time
{
    void markFrame();
    void init();

    float delta();
    float deltaMs();
    u64 ticks();
    float sinceLoad();

    float onePercentMs();
    float averageDelta();
    float averageDeltaMs();
    float fps();
}
