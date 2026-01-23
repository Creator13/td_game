#pragma once

#include "datatype.h"

namespace core::time
{
    void markFrame();
    void init();

    float delta();
    u64 ticks();
    float sinceLoad();

    float onePercent();
    float averageDelta();
    float fps();
}
