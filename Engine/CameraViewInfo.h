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
	};
}
