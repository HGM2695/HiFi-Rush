#pragma once

#include "AnimationTypes.h"

namespace gm
{
    class AnimationClip;

    class AnimationController
    {
    public:
        bool            Play(const AnimationClip& clip, const AnimationPlayOption& option = {});
        void            Reset();
        void            Pause();
        void            Resume();
        void            Tick(float deltaTime);

        float           GetPlayTime() const { return _currentTime; }
        AnimationState  GetState() const { return _state; }
        bool            IsPlaying() const { return _state == AnimationState::Playing; }
        bool            IsPaused() const { return _state == AnimationState::Paused; }
        bool            IsLoop() const { return _isLoop; }

    private:
        AnimationState  _state = AnimationState::Init;
        float           _currentTime = 0.f;
        float           _clipLength = 0.f;
        bool            _isLoop = true;
    };
}
