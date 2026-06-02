#pragma once

#include <optional>
#include <functional>
#include <string>

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
        float startTime = 0.f;
        std::optional<bool> loopOverride;
        std::optional<float> playRateOverride;
    };

    using AnimationNotifyListener = std::function<void(const std::wstring&)>;
}
