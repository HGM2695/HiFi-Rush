#pragma once

#include "MathTypes.h"
#include "Types.h"

namespace gm
{
	class GameObject;
	class Resources;
	class Scene;
	class SocketComponent;

	struct QamilSpawnDesc
	{
		Matrix	world = Matrix::Identity;
		int32	maxHealth = 100;
	};

	class QamilSpawner
	{
	public:
		explicit QamilSpawner(Resources& resources);

		GameObject* Spawn(Scene& scene, const QamilSpawnDesc& desc) const;

	private:
		bool AddSphereHurtBox(GameObject& qamil, SocketComponent& sockets, const wchar_t* colliderId, const wchar_t* socketName, const wchar_t* boneName, const Vector3& localCenter, float radius, bool isTrigger) const;
		bool AddBoxHurtBox(GameObject& qamil, SocketComponent& sockets, const wchar_t* colliderId, const wchar_t* socketName, const wchar_t* boneName, const Vector3& localCenter, const Vector3& size, bool isTrigger) const;

	private:
		Resources& _resources;
	};
}
