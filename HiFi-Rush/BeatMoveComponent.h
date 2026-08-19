#pragma once

#include "Component.h"
#include "TriggerBinding.h"

namespace gm
{
	class BeatSystem;
	class TransformComponent;

	struct BeatMoveDesc
	{
		std::wstring	triggerId{};
		float			beatOffset = 0.f;
		Vector3			targetPosition{};
		float			durationBeats = 1.f;
	};

	class BeatMoveComponent : public Component
	{
	public:
		BeatMoveComponent(const BeatSystem& beatSystem, const BeatMoveDesc& desc);

		bool IsActive() const { return _state != MoveState::Inactive; }

	protected:
		void OnInitialize() override;
		void OnTick(float deltaTime) override;

	private:
		enum class MoveState
		{
			Inactive,
			Scheduled,
		};

	private:
		void Schedule(float startBeat);
		void ResetAction();

	private:
		const BeatSystem&		_beatSystem;
		BeatMoveDesc			_desc{};
		TriggerBinding			_triggerBinding{};
		TransformComponent*		_transform = nullptr;
		Vector3					_initialPosition{};
		Vector3					_startPosition{};
		float					_startBeat = 0.f;
		MoveState				_state = MoveState::Inactive;
	};
}
