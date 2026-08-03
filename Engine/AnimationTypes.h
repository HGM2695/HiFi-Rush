#pragma once

#include <optional>

namespace gm
{
    enum class AnimationState
    {
        Init,
        Playing,
        Paused,
        Completed,
    };

    struct AnimationPlayOption
    {
        float                   startTime = 0.f;
        float                   blendDuration = 0.f;
        std::optional<bool>     loopOverride;
        std::optional<float>    playRateOverride;
    };
}
