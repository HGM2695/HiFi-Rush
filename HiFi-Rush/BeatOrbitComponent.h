#pragma once

#include "Component.h"

namespace gm
{
	class BeatSystem;
	class TransformComponent;

	struct BeatOrbitDesc
	{
		Vector3	center{};
		float	evenBeatDeltaDegrees = -10.f;
		float	oddBeatDeltaDegrees = -5.f;
		float	interpolationSpeed = 13.f;
		bool	faceCenter = false;
	};

	class BeatOrbitComponent : public Component
	{
	public:
		BeatOrbitComponent(const BeatSystem& beatSystem, const BeatOrbitDesc& desc);

	protected:
		void OnInitialize() override;
		void OnTick(float deltaTime) override;

	private:
		void UpdateTargetAngle();
		void ApplyTransform();

	private:
		const BeatSystem&	_beatSystem;
		BeatOrbitDesc		_desc{};
		TransformComponent*	_transform = nullptr;
		float				_radius = 0.f;
		float				_heightOffset = 0.f;
		float				_currentOrbitAngle = 0.f;
		float				_targetOrbitAngle = 0.f;
	};
}
