#pragma once

#include "EnvironmentComponentFactory.h"

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
		bool SpawnObject(GameplayScene& scene, const EnvironmentObjectData& objectData) const;

	private:
		Resources&					_resources;
		EnvironmentComponentFactory	_componentFactory;
	};
}
