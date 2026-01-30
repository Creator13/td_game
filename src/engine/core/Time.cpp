#include "core/Time.h"

#include <plf_nanotimer.h>
#include "math/func.h"

namespace core::time
{
    namespace
    {
        constexpr uint32_t FRAMETIME_AGG_SIZE = 256;

        struct GameTime
        {
            GameTime();

            plf::nanotimer globalTimer;
            plf::nanotimer frameTimer;

            uint64_t frameCount;

            float aggregator[FRAMETIME_AGG_SIZE];
            uint32_t aggPtr;

            void start();
            void markFrame();

            float getLatestDelta() const;
            float getAverageDelta() const;
            float getMaxDelta() const;

        private:
            void add_measurement(float time);
        };

        GameTime::GameTime() : frameCount(0), aggPtr(0)
        {
            for (uint32_t i = 0; i < FRAMETIME_AGG_SIZE; i++)
            {
                aggregator[i] = 0;
            }
        }

        void GameTime::start()
        {
            globalTimer.start();
            frameTimer.start();
        }

        void GameTime::markFrame()
        {
            add_measurement(frameTimer.get_elapsed_ms());

            frameTimer.start();
            frameCount++;
        }

        float GameTime::getLatestDelta() const
        {
            return aggregator[aggPtr];
        }

        float GameTime::getAverageDelta() const
        {
            float total = 0;
            for (uint32_t i = 0; i < FRAMETIME_AGG_SIZE; i++)
            {
                total += aggregator[i];
            }
            return total / static_cast<float>(FRAMETIME_AGG_SIZE);
        }

        float GameTime::getMaxDelta() const
        {
            float max = 0;
            for (uint32_t i = 0; i < FRAMETIME_AGG_SIZE; i++)
            {
                max = math::max(max, aggregator[i]);
            }
            return max;
        }

        void GameTime::add_measurement(float time)
        {
            aggPtr = (aggPtr + 1) % FRAMETIME_AGG_SIZE;
            aggregator[aggPtr] = time;
        }

        constexpr float msToSec(float in)
        {
            return in / 1000.f;
        }

        GameTime globalTime;
    }

    void markFrame()
    {
        globalTime.markFrame();
    }

    void init()
    {
        globalTime.start();
    }

    float delta()
    {
        return msToSec(globalTime.getLatestDelta());
    }

    uint64_t ticks()
    {
        return globalTime.frameCount;
    }

    float sinceLoad()
    {
        return msToSec(globalTime.globalTimer.get_elapsed_ms());
    }

    float onePercent()
    {
        return msToSec(globalTime.getMaxDelta());
    }

    float averageDelta()
    {
        return msToSec(globalTime.getAverageDelta());
    }

    float fps()
    {
        return 1.f / averageDelta();
    }
}
