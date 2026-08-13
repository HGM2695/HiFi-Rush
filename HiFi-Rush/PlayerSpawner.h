#pragma once

#include "MathTypes.h"

namespace gm
{
	class GameObject;
	class Resources;
	class Scene;
	struct PlayerRuntimeState;

	inline constexpr wchar_t PlayerCameraKey[] = L"PlayerCamera";

	struct PlayerSpawnDesc
	{
		Vector3	position{};
		float	rotationY = 0.f;
		float	cameraDistance = 3.5f;
		float	cameraYaw = 0.f;
		float	cameraPitch = 0.f;
		float	cameraHeight = 0.f;
	};

	class PlayerSpawner
	{
	public:
		explicit PlayerSpawner(Resources& resources);

		GameObject* Spawn(Scene& scene, const PlayerSpawnDesc& desc, PlayerRuntimeState& runtimeState) const;

	private:
		Resources& _resources;
	};
}
