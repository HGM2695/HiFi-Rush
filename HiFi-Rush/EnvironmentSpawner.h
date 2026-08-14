#pragma once

#include "EnvironmentComponentFactory.h"
#include "WeakGameObjectPtr.h"

#include <vector>

namespace gm
{
	class GameplayScene;
	class Resources;
	struct EnvironmentObjectData;

	class EnvironmentSpawner
	{
	public:
		explicit EnvironmentSpawner(Resources& resources);

		bool Spawn(GameplayScene& scene, const std::vector<EnvironmentObjectData>& objectDatas) const;

	private:
		struct SpawnEntry
		{
			WeakGameObjectPtr						owner{};
			std::vector<EnvironmentTriggerAction>	triggerActions{};
		};

		bool BuildTriggerSequences(GameplayScene& scene, const std::vector<SpawnEntry>& spawnEntries) const;
		bool SpawnObject(GameplayScene& scene, const EnvironmentObjectData& objectData, SpawnEntry& outSpawnEntry) const;

	private:
		Resources&					_resources;
		EnvironmentComponentFactory	_componentFactory;
	};
}
