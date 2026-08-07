#pragma once

#include "IBeatTriggerAction.h"
#include "Component.h"

#include <string>

namespace gm
{
	class BeatSystem;
	class SkeletalAnimatorComponent;

	struct BeatTriggeredSkeletalAnimationDesc
	{
		std::wstring clipName = L"Default";
	};

	class BeatTriggeredSkeletalAnimationComponent : public Component, public IBeatTriggerAction
	{
	public:
		BeatTriggeredSkeletalAnimationComponent(const BeatSystem& beatSystem, SkeletalAnimatorComponent& animator, BeatTriggeredSkeletalAnimationDesc desc);

		virtual TickGroup GetTickGroup() const override { return TickGroup::GameLogic; }

		void Schedule(float startBeat) override;
		void Reset() override;

	protected:
		void OnInitialize() override;
		void OnTick(float deltaTime) override;

	private:
		enum class PlaybackState
		{
			Inactive,
			Scheduled,
			Playing,
		};

	private:
		const BeatSystem&				_beatSystem;
		SkeletalAnimatorComponent&		_animator;
		BeatTriggeredSkeletalAnimationDesc _desc{};
		float							_startBeat = 0.f;
		PlaybackState					_state = PlaybackState::Inactive;
	};
}
