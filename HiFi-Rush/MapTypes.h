#pragma once

#include "EnvironmentComponentTypes.h"
#include "FogTypes.h"
#include "LightTypes.h"
#include "MaterialTypes.h"
#include "MathTypes.h"
#include "MonsterTypes.h"
#include "ToneMappingTypes.h"
#include "Types.h"

#include <vector>

namespace gm
{
	struct MaterialColorOverrideData
	{
		uint32				materialSlot = 0;
		MaterialColorData	colorData{};
	};

	struct EnvironmentObjectData
	{
		std::vector<EnvironmentComponentData>	components{};
		std::vector<MaterialColorOverrideData>	materialColorOverrides{};
		bool								hasRenderMesh = false;
		uint32									modelIndex = 0;
		Matrix									world = Matrix::Identity;
	};

	struct MapData
	{
		std::vector<EnvironmentObjectData>	objects;
		std::vector<MonsterSpawnData>		monsterSpawnDatas;
		SceneAmbientSettings				ambientSettings{};
		DepthFogSettings					depthFogSettings{};
		ToneMappingSettings				toneMappingSettings{};
	};
}
