#pragma once

#include "Component.h"
#include "TriggerBinding.h"

namespace gm
{
	class BeatSystem;
	class TransformComponent;

	struct BeatTriggeredRotationDesc
	{
		std::wstring	triggerId{};
		float			beatOffset = 0.f;
		Vector3			axis{ 0.f, 0.f, 1.f };
		float			angleDegrees = 90.f;
		float			durationBeats = 0.25f;
	};

	class BeatTriggeredRotationComponent : public Component
	{
	public:
		BeatTriggeredRotationComponent(const BeatSystem& beatSystem, const BeatTriggeredRotationDesc& desc);

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
		void Schedule(float startBeat);
		void ResetAction();

	private:
		const BeatSystem&			_beatSystem;
		BeatTriggeredRotationDesc	_desc{};
		TriggerBinding				_triggerBinding{};
		TransformComponent*			_transform = nullptr;
		Vector3						_rotationAxis{};
		Quaternion					_initialRotation{};
		Quaternion					_startRotation{};
		Quaternion					_targetRotation{};
		float						_startBeat = 0.f;
		RotationState				_state = RotationState::Inactive;
	};
}
