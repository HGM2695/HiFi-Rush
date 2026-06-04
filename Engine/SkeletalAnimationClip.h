#pragma once

#include "AnimationClip.h"
#include "MeshTypes.h"

namespace gm
{
	class SkeletalAnimationClip : public AnimationClip
	{
	public:
		static std::shared_ptr<SkeletalAnimationClip> Create(const SkeletalAnimationClipData& desc);
		virtual ~SkeletalAnimationClip();

		static constexpr ResourceType Type = ResourceType::SkeletalAnimationClip;
		virtual ResourceType GetType() const override { return Type; }

		const std::wstring&							GetClipName() const { return _name; }
		float										GetDurationTicks() const { return _durationTicks; }
		float										GetTicksPerSecond() const { return _ticksPerSecond; }
		const std::vector<AnimationChannelData>&	GetChannels() const { return _channels; }
		const AnimationChannelData*					FindChannel(uint32 boneIndex) const;

	private:
		SkeletalAnimationClip() = default;

	private:
		std::wstring						_name;
		float								_durationTicks = 0.f;
		float								_ticksPerSecond = 0.f;
		std::vector<AnimationChannelData>	_channels;
	};
}
