#pragma once

#include "IBeatTriggerAction.h"
#include "Component.h"

namespace gm
{
	class BeatSystem;
	class TransformComponent;

	struct BeatMoveDesc
	{
		Vector3	targetPosition{};
		float	durationBeats = 1.f;
	};

	class BeatMoveComponent : public Component, public IBeatTriggerAction
	{
	public:
		BeatMoveComponent(const BeatSystem& beatSystem, const BeatMoveDesc& desc);

		void Activate();
		void Schedule(float startBeat) override;
		void Reset() override;
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
		const BeatSystem&		_beatSystem;
		BeatMoveDesc			_desc{};
		TransformComponent*		_transform = nullptr;
		Vector3					_initialPosition{};
		Vector3					_startPosition{};
		float					_startBeat = 0.f;
		MoveState				_state = MoveState::Inactive;
	};
}
