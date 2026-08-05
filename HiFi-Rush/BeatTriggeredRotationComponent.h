#pragma once

#include "Component.h"
#if GM_ENABLE_DEBUG_TOOLS
#include "Event.h"
#endif

namespace gm
{
	class BeatSystem;
	class TransformComponent;

	struct BeatTriggeredRotationDesc
	{
		Vector3	axis{ 0.f, 0.f, 1.f };
		float	angleDegrees = 90.f;
		float	durationBeats = 0.25f;
	};

	class BeatTriggeredRotationComponent : public Component
	{
	public:
		BeatTriggeredRotationComponent(const BeatSystem& beatSystem, const BeatTriggeredRotationDesc& desc);

		void Activate();
		void Reset();
		bool IsActive() const { return _state == RotationState::Rotating; }

	protected:
		void OnInitialize() override;
		void OnTick(float deltaTime) override;

	private:
		enum class RotationState
		{
			Inactive,
			Rotating,
			Completed,
		};

	private:
		const BeatSystem&			_beatSystem;
		BeatTriggeredRotationDesc	_desc{};
#if GM_ENABLE_DEBUG_TOOLS
		EventConnection				_debugEventConnection{};
#endif
		TransformComponent*			_transform = nullptr;
		Vector3						_rotationAxis{};
		Quaternion					_initialRotation{};
		Quaternion					_startRotation{};
		Quaternion					_targetRotation{};
		float						_startBeat = 0.f;
		RotationState				_state = RotationState::Inactive;
	};
}
