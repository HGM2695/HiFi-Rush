#pragma once

#include "EngineCore.h"
#include "MathUtil.h"

namespace gm
{
	struct CameraViewInfo
	{
		Vector3		position{};
		Quaternion	rotation{};
		Matrix		view = Math::IdentityMatrix();
		Matrix		projection = Math::IdentityMatrix();
		float		nearPlane = 0.1f;
		float		farPlane = 1000.f;
	};
}
