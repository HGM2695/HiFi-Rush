#include "AnimationController.h"
#include "AnimationClip.h"
#include "GMAssert.h"

#include <algorithm>
#include <cmath>

namespace gm
{
    bool AnimationController::Play(const AnimationClip& clip, const AnimationPlayOption& option)
    {
        _clipLength = clip.GetLength();
        _isLoop = option.loopOverride.value_or(clip.IsLoop());
        if (option.playRateOverride.has_value())
            SetPlayRate(option.playRateOverride.value());

        _state = AnimationState::Playing;
        _currentTime = std::max(0.0f, option.startTime);
        _hasLooped = false;

        return true;
    }

    void AnimationController::Reset()
    {
        _state = AnimationState::Init;
        _currentTime = 0.f;
        _hasLooped = false;
    }

    void AnimationController::Pause()
    {
        if (_state == AnimationState::Playing)
            _state = AnimationState::Paused;
    }

    void AnimationController::Resume()
    {
        if (_state == AnimationState::Paused)
            _state = AnimationState::Playing;
    }

    void AnimationController::Tick(float deltaTime)
    {
        _hasLooped = false;

        if (_state != AnimationState::Playing)
            return;

        _currentTime += deltaTime * _playRate;

        if (_currentTime < _clipLength)
            return;

        if (_isLoop)
        {
            _hasLooped = true;
            _currentTime = std::fmod(_currentTime, _clipLength);
            return;
        }

        _currentTime = std::nextafter(_clipLength, 0.0f);
        _state = AnimationState::Completed;
    }

    void AnimationController::SetPlayRate(float playRate)
    {
        GM_ASSERT_RETURN(playRate >= 0.f, "Animation 재생 속도는 0 이상이어야 합니다.");
        _playRate = playRate;
    }

    void AnimationController::SetPlayTime(float playTime)
    {
        GM_ASSERT_RETURN(playTime >= 0.f, "Animation 재생 시각은 0 이상이어야 합니다.");
        _hasLooped = false;

        if (_clipLength <= 0.f)
        {
            _currentTime = 0.f;
            return;
        }

        if (_isLoop)
        {
            const float nextTime = std::fmod(playTime, _clipLength);
            _hasLooped = nextTime < _currentTime;
            _currentTime = nextTime;
            return;
        }

        _currentTime = std::min(playTime, std::nextafter(_clipLength, 0.f));
    }
}
