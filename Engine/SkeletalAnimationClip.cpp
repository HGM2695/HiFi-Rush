#include "SkeletalAnimationClip.h"

#include <algorithm>

namespace gm
{
	std::shared_ptr<SkeletalAnimationClip> SkeletalAnimationClip::Create(const SkeletalAnimationClipData& data)
	{
		GM_ASSERT_RETURN_VAL(data.channels.empty() == false, nullptr, "SkeletalAnimationClip Channel 데이터가 비어 있습니다.");

		std::shared_ptr<SkeletalAnimationClip> clip(new SkeletalAnimationClip());
		clip->_name = data.name;
		clip->_durationTicks = data.duration;
		clip->_ticksPerSecond = data.ticksPerSecond;
		clip->_channels = data.channels;
		clip->_length = data.ticksPerSecond > 0.f ? data.duration / data.ticksPerSecond : data.duration;
		clip->_frameCount = 0;

		for (const AnimationNotifyData& notifyEvent : data.notifyEvents)
			clip->AddNotify(notifyEvent.time, notifyEvent.name);

		for (const AnimationChannelData& channel : clip->_channels)
		{
			clip->_frameCount = std::max(clip->_frameCount, static_cast<uint32>(channel.keyFrames.size()));
		}

		GM_ASSERT_RETURN_VAL(clip->_length > 0.f, nullptr, "SkeletalAnimationClip 길이가 0입니다.");
		GM_ASSERT_RETURN_VAL(clip->_frameCount > 0, nullptr, "SkeletalAnimationClip KeyFrame 데이터가 비어 있습니다.");

		return clip;
	}

	SkeletalAnimationClip::~SkeletalAnimationClip() = default;

	const AnimationChannelData* SkeletalAnimationClip::FindChannel(uint32 boneIndex) const
	{
		for (const AnimationChannelData& channel : _channels)
		{
			if (channel.boneIndex == boneIndex)
				return &channel;
		}

		return nullptr;
	}
}
