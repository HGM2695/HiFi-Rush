#include "AnimationController.h"
#include "SpriteAnimationClip.h"
#include "GMAssert.h"
#include <algorithm>
#include <cmath>

namespace gm
{
    bool AnimationController::Play(const AnimationClip& clip, const AnimationPlayOption& option)
    {
        _clipLength = clip.GetLength();
        _isLoop = option.loopOverride.value_or(clip.IsLoop());
        _state = AnimationState::Playing;
        _currentTime = std::max(0.0f, option.startTime);

        return true;
    }

    void AnimationController::Reset()
    {
        _state = AnimationState::Init;
        _currentTime = 0.f;
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

    void AnimationController::Update(float deltaTime)
    {
        if (_state != AnimationState::Playing)
            return;

        _currentTime += deltaTime;

        if (_currentTime < _clipLength)
            return;

        if (_isLoop)
        {
            _currentTime = std::fmod(_currentTime, _clipLength);
            return;
        }

        _currentTime = std::nextafter(_clipLength, 0.0f);
        _state = AnimationState::Completed;
    }
}
