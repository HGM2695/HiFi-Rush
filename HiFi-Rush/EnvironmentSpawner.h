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

	private:
		bool SpawnObject(Scene& scene, const EnvironmentObjectData& objectData) const;

	private:
		Resources& _resources;
	};
}
