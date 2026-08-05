#pragma once

#include "Component.h"

namespace gm
{
	class BeatSystem;
	class SkeletalAnimatorComponent;

	struct BeatSkeletalAnimationSyncDesc
	{
		float cycleBeats = 1.f;
		float phaseOffsetBeats = 0.f;
	};

	class BeatSkeletalAnimationSyncComponent : public Component
	{
	public:
		BeatSkeletalAnimationSyncComponent(const BeatSystem& beatSystem, SkeletalAnimatorComponent& animator, const BeatSkeletalAnimationSyncDesc& desc);

		virtual TickGroup GetTickGroup() const override { return TickGroup::GameLogic; }

	protected:
		void OnInitialize() override;
		void OnTick(float deltaTime) override;

	private:
		const BeatSystem&				_beatSystem;
		SkeletalAnimatorComponent&		_animator;
		BeatSkeletalAnimationSyncDesc	_desc{};
	};
}
