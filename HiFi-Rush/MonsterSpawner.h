#pragma once

#include "WeakGameObjectPtr.h"

#include <string>
#include <vector>

namespace gm
{
	class GameObject;
	class Resources;
	class Scene;
	struct MonsterSpawnData;

	struct MonsterSpawnResult
	{
		WeakGameObjectPtr	monster{};
		std::wstring		activationTriggerId{};
	};

	class MonsterSpawner
	{
	public:
		explicit MonsterSpawner(Resources& resources);

		bool		SpawnAll(Scene& scene, const std::vector<MonsterSpawnData>& spawnDataList, std::vector<MonsterSpawnResult>& outSpawnResults) const;
		GameObject* Spawn(Scene& scene, const MonsterSpawnData& data) const;

	private:
		bool AddCommonComponents(GameObject& monster, const MonsterSpawnData& data) const;
		bool AddActivationComponents(GameObject& monster, const MonsterSpawnData& data) const;

	private:
		Resources& _resources;
	};
}
