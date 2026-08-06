#pragma once

#include "Component.h"

#include <string>
#include <unordered_map>

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
		BeatSkeletalAnimationSyncComponent(const BeatSystem& beatSystem, SkeletalAnimatorComponent& animator, const BeatSkeletalAnimationSyncDesc& defaultDesc);

		virtual TickGroup GetTickGroup() const override { return TickGroup::GameLogic; }

		bool AddClipSyncRule(const std::wstring& clipName, const BeatSkeletalAnimationSyncDesc& desc);

	protected:
		void OnInitialize() override;
		void OnTick(float deltaTime) override;

	private:
		const BeatSkeletalAnimationSyncDesc* FindSyncDesc() const;

	private:
		std::unordered_map<std::wstring, BeatSkeletalAnimationSyncDesc> _clipSyncRules;
		const BeatSystem&				_beatSystem;
		SkeletalAnimatorComponent&		_animator;
		BeatSkeletalAnimationSyncDesc	_defaultDesc{};
	};
}
