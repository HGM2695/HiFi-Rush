#pragma once

#include "EnvironmentComponentFactory.h"
#include "WeakGameObjectPtr.h"

#include <vector>

namespace gm
{
	class Resources;
	class Scene;
	struct EnvironmentObjectData;
	struct MapData;

	class EnvironmentSpawner
	{
	public:
		explicit EnvironmentSpawner(Resources& resources);

		bool Spawn(Scene& scene, const MapData& mapData) const;

	private:
		struct SpawnEntry
		{
			WeakGameObjectPtr						owner{};
			std::vector<EnvironmentTriggerAction>	triggerActions{};
		};

		bool BuildTriggerSequences(Scene& scene, const std::vector<SpawnEntry>& spawnEntries) const;
		bool SpawnObject(Scene& scene, const EnvironmentObjectData& objectData, SpawnEntry& outSpawnEntry) const;

	private:
		Resources&					_resources;
		EnvironmentComponentFactory	_componentFactory;
	};
}
