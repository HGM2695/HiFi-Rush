#pragma once

#include "Component.h"
#include "TriggerBinding.h"

namespace gm
{
	class BeatSystem;
	class TransformComponent;

	struct BeatTriggeredRotationShakeDesc
	{
		std::wstring	triggerId{};
		float			beatOffset = 0.f;
		Vector3			axis{ 1.f, 0.f, 0.f };
		float			angleDegrees = 15.f;
		float			durationBeats = 0.75f;
	};

	class BeatTriggeredRotationShakeComponent final : public Component
	{
	public:
		BeatTriggeredRotationShakeComponent(const BeatSystem& beatSystem, const BeatTriggeredRotationShakeDesc& desc);

		bool IsShaking() const { return _isShaking; }

	protected:
		void OnInitialize() override;
		void OnTick(float deltaTime) override;

	private:
		void StartShake(float startBeat);
		void ResetShake();
		float EvaluateAngle(float progress) const;

	private:
		const BeatSystem&					_beatSystem;
		BeatTriggeredRotationShakeDesc	_desc{};
		TriggerBinding						_triggerBinding{};
		TransformComponent*					_transform = nullptr;
		Vector3								_axis{};
		Quaternion							_initialRotation{};
		float								_startBeat = 0.f;
		bool								_isShaking = false;
	};
}
