#pragma once

#include "MathTypes.h"

namespace gm
{
	class Scene;

	void RegisterSceneDebugTools();
	void TickSceneTransitionDebug();
	void CreateEnvironmentOverviewCamera(Scene& scene, const Vector3& cameraPosition, const Vector3& targetPosition);
	void ActivateEnvironmentOverviewCamera(Scene& scene);
}
