#pragma once

#include "EngineCore.h"
#include "AnimationClip.h"
#include "SpriteFrame.h"
#include <memory>
#include <vector>

namespace gm
{
    class Texture;

    struct SpriteAnimationClipDesc
    {
        std::wstring path;
    };

    class SpriteAnimationClip : public AnimationClip
    {
    public:
        static std::shared_ptr<SpriteAnimationClip> Create(const SpriteAnimationClipDesc& desc);
        virtual ~SpriteAnimationClip();

        static constexpr ResourceType Type = ResourceType::SpriteAnimationClip;
        virtual	ResourceType	    GetType() const override { return Type; }


        void                        SetTexture(const std::shared_ptr<Texture>& texture) { _texture = texture; }
        std::shared_ptr<Texture>    GetTexture() const { return _texture; }
        uint32                      GetFrameCount() const { return static_cast<uint32>(_frameList.size()); }
        void                        AddFrame(const SpriteFrame& frame);

        const SpriteFrame&          GetFrame(size_t index) const { return _frameList[index]; }
        uint32                      GetFrameIndexByTime(float time) const;
        const SpriteFrame&          GetFrameByTime(float time) { return GetFrame(GetFrameIndexByTime(time)); }

    private:
        SpriteAnimationClip();

    private:
        std::shared_ptr<Texture>     _texture;
        std::vector<SpriteFrame>     _frameList;
    };
}
