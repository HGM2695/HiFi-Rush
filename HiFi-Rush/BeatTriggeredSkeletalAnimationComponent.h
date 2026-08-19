#pragma once

#include "Component.h"
#include "TriggerBinding.h"

#include <string>
#include <vector>

namespace gm
{
	class BeatSystem;
	class Collider3DComponent;
	class SkeletalAnimatorComponent;

	struct BeatTriggeredSkeletalAnimationDesc
	{
		std::wstring	triggerId{};
		float			beatOffset = 0.f;
		std::wstring	clipName = L"Default";
		bool			initiallyVisible = false;
		bool			hideWhenCompleted = true;
		bool			disableCollidersWhenCompleted = false;
	};

	class BeatTriggeredSkeletalAnimationComponent : public Component
	{
	public:
		BeatTriggeredSkeletalAnimationComponent(const BeatSystem& beatSystem, SkeletalAnimatorComponent& animator, BeatTriggeredSkeletalAnimationDesc desc);

		virtual TickGroup GetTickGroup() const override { return TickGroup::GameLogic; }

	protected:
		void OnInitialize() override;
		void OnTick(float deltaTime) override;

	private:
		struct ColliderState
		{
			Collider3DComponent*	collider = nullptr;
			bool				wasEnabled = true;
		};

	private:
		enum class PlaybackState
		{
			Inactive,
			Scheduled,
			Playing,
		};

	private:
		void Schedule(float startBeat);
		void ResetAction();

	private:
		const BeatSystem&				_beatSystem;
		SkeletalAnimatorComponent&		_animator;
		BeatTriggeredSkeletalAnimationDesc _desc{};
		TriggerBinding					_triggerBinding{};
		std::vector<ColliderState>		_colliderStates{};
		float							_startBeat = 0.f;
		PlaybackState					_state = PlaybackState::Inactive;
	};
}
