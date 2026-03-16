#pragma once

#include "AnimationClip.h"
#include "SpriteFrame.h"
#include <memory>
#include <vector>

namespace gm
{
    class Texture;

    class SpriteAnimationClip : public AnimationClip
    {
    public:
        SpriteAnimationClip();
        virtual ~SpriteAnimationClip();

        static constexpr ResourceType Type = ResourceType::SpriteAnimationClip;
        virtual	ResourceType	    GetType() const override { return Type; }
        virtual uint32_t            GetFrameIndexByTime(float time) const override;

        void                        SetTexture(const std::shared_ptr<Texture>& texture) { _texture = texture; }
        std::shared_ptr<Texture>    GetTexture() const { return _texture; }
        uint32_t                    GetFrameCount() const { return static_cast<uint32_t>(_frameList.size()); }
        void                        AddFrame(const SpriteFrame& frame);
        const SpriteFrame&          GetFrame(size_t index) const { return _frameList[index]; }
        const SpriteFrame&          GetFrameByTime(float time) { return GetFrame(GetFrameIndexByTime(time)); }

    protected:
        virtual bool        LoadInternal(const std::wstring& path) override;

    private:
        std::shared_ptr<Texture>     _texture;
        std::vector<SpriteFrame>     _frameList;
    };
}
