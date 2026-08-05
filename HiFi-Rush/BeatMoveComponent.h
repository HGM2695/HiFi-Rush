#pragma once

#include "Component.h"
#if GM_ENABLE_DEBUG_TOOLS
#include "Event.h"
#endif

namespace gm
{
	class BeatSystem;
	class TransformComponent;

	struct BeatMoveDesc
	{
		Vector3	targetPosition{};
		float	durationBeats = 1.f;
	};

	class BeatMoveComponent : public Component
	{
	public:
		BeatMoveComponent(const BeatSystem& beatSystem, const BeatMoveDesc& desc);

		void Activate();
		void Reset();
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

		void ScheduleMove();

	private:
		const BeatSystem&	_beatSystem;
		BeatMoveDesc		_desc{};
#if GM_ENABLE_DEBUG_TOOLS
		EventConnection		_debugEventConnection{};
#endif
		TransformComponent*		_transform = nullptr;
		Vector3					_initialPosition{};
		Vector3					_startPosition{};
		float					_startBeat = 0.f;
		MoveState				_state = MoveState::Inactive;
	};
}
