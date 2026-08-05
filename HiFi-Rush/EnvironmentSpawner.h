#pragma once

namespace gm
{
	class Resources;
	class Scene;
	struct EnvironmentMapData;
	struct EnvironmentObjectData;

	class EnvironmentSpawner
	{
	public:
		explicit EnvironmentSpawner(Resources& resources);

		bool Spawn(Scene& scene, const EnvironmentMapData& mapData) const;
		bool SpawnTriggerObjects(Scene& scene, const EnvironmentMapData& mapData) const;

	private:
		bool SpawnObjects(Scene& scene, const EnvironmentMapData& mapData, bool isTriggerObject) const;
		bool SpawnObject(Scene& scene, const EnvironmentObjectData& objectData, bool isTriggerObject) const;

	private:
		Resources& _resources;
	};
}
