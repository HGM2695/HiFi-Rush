#pragma once

#include "Component.h"
#include "Event.h"

#include <memory>
#include <string>
#include <vector>

namespace gm
{
	class HurtBoxComponent;
	class SkeletalAnimatorComponent;
	class TriggerSystem;
	struct HitEvent;

	class HitReactionComponent final : public Component
	{
	public:
		HitReactionComponent(const std::wstring& completionTriggerId, const std::wstring& reactionAnimationClipName);

		TickGroup GetTickGroup() const override { return TickGroup::GameLogic; }

	protected:
		void OnInitialize() override;
		void OnTick(float deltaTime) override;

	private:
		void HandleHurt(const HitEvent& event);
		void ActivateTrigger();

		enum class State
		{
			WaitingForHit,
			PlayingAnimation,
			Completed,
		};

		std::vector<std::unique_ptr<EventConnection>>	_hurtConnections{};
		std::wstring									_completionTriggerId{};
		std::wstring									_reactionAnimationClipName{};
		SkeletalAnimatorComponent*						_animator = nullptr;
		TriggerSystem*									_triggerSystem = nullptr;
		State											_state = State::WaitingForHit;
	};
}
