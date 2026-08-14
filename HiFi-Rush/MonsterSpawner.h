#pragma once

#include <vector>

namespace gm
{
	class GameObject;
	class Resources;
	class Scene;
	struct MonsterSpawnData;

	class MonsterSpawner
	{
	public:
		explicit MonsterSpawner(Resources& resources);

		bool		Spawn(Scene& scene, const std::vector<MonsterSpawnData>& spawnDataList) const;
		GameObject* Spawn(Scene& scene, const MonsterSpawnData& data) const;

	private:
		Resources& _resources;
	};
}
