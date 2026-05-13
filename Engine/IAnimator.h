#pragma once

#include "AnimationTypes.h"

namespace gm
{
    class IAnimator
    {
    public:
        virtual ~IAnimator() = default;

        virtual bool			HasClip(const std::wstring& name) const = 0;
        virtual bool			Play(const std::wstring& name, const AnimationPlayOption& option = {}) = 0;

        virtual void            Reset() = 0;
        virtual void            Pause() = 0;
        virtual void            Resume() = 0;

        virtual AnimationState  GetState() const = 0;
        virtual float           GetPlayTime() const = 0;
        virtual bool            IsLoop() const = 0;
    };
}
