#pragma once

#include "Component.h"

namespace gm
{
	class TransformComponent;

	struct ContinuousRotationDesc
	{
		Vector3	axis{ 0.f, 1.f, 0.f };
		float	angularSpeedDegrees = 0.f;
	};

	class ContinuousRotationComponent : public Component
	{
	public:
		explicit ContinuousRotationComponent(const ContinuousRotationDesc& desc);

	protected:
		void OnInitialize() override;
		void OnTick(float deltaTime) override;

	private:
		ContinuousRotationDesc	_desc{};
		TransformComponent*		_transform = nullptr;
		Vector3					_rotationAxis{};
	};
}
